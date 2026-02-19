
#include "RenderObject.h"
#include "Application.h"
#include "Utils.h"

#include <array>
#include <queue>
#include <stack>
#include <iostream>

using App = Application;

using glm::vec3;
using glm::mat4;


/********************************* RenderObject *********************************/

std::vector<std::weak_ptr<RenderObject>> RenderObject::worldList;
std::vector<std::weak_ptr<RenderObject>> RenderObject::viewList;
std::vector<std::weak_ptr<RenderObject>> RenderObject::screenList;
std::shared_ptr<RenderObject> RenderObject::newObject;

EventPack<int, void, const std::shared_ptr<RenderObject>&> RenderObject::setDefaultStat;

void RenderObject::SortScreenList() {
	std::array<std::vector<std::weak_ptr<RenderObject>>, MAX_UI_LAYERS> bucketList;
	static constexpr int MAX_UI_LAYERS_LESS_ONE = MAX_UI_LAYERS - 1;
	int prevLayer = MAX_UI_LAYERS_LESS_ONE;
	bool inOrder = true;

	for (auto it = screenList.rbegin(); it != screenList.rend(); it++) {
		int thisLayer = it->lock()->UILayer;

		thisLayer = Clamp(thisLayer, 0, MAX_UI_LAYERS_LESS_ONE);

		if (prevLayer < thisLayer) {
			inOrder = false;
			break;
		}
		prevLayer = thisLayer;
	}
	if (inOrder)
		return;

	for (auto& obj_weak : screenList) {
		int thisLayer = obj_weak.lock()->UILayer;

		thisLayer = Clamp(thisLayer, 0, MAX_UI_LAYERS_LESS_ONE);
		bucketList[thisLayer].push_back(obj_weak);
	}

	screenList.clear();
	for (unsigned layer = 0; layer < bucketList.size(); layer++)
		for (auto& obj : bucketList[layer])
			screenList.push_back(obj);
}

void RenderObject::UpdateModel() {
	if (isDirty || trl != prevTrl || rot != prevRot || scl != prevScl) {
		prevTrl = trl;
		prevRot = rot;
		prevScl = scl;
		isDirty = false;
		model = GetModel();
		for (auto child : children) {
			child->UpdateModelWithParent(model);
		}
	}
}

void RenderObject::Destroy() {
	if (auto shared_parent = parent.lock()) {
		for (unsigned i = 0; i < shared_parent->children.size(); i++) {
			auto& p_child = shared_parent->children[i];
			if (p_child.get() == this) {
				shared_parent->children.erase(shared_parent->children.begin() + i);
				break;
			}
		}
	}
}

void RenderObject::NewChild(std::shared_ptr<RenderObject> child) {
	children.push_back(std::move(child));
	children.back()->parent = shared_from_this();
	newObject = children.back();
	AddHierarchyToList(renderType, newObject);
	newObject->UpdateModel();
}

void RenderObject::SwapParentTo(const std::shared_ptr<RenderObject>& newParent) {
	auto thisShared = shared_from_this();
	Destroy(); // disconnect from parent

	std::queue<std::shared_ptr<RenderObject>> queue;
	queue.push(thisShared);

	auto removeFrom = [&](std::vector<std::weak_ptr<RenderObject>>& list) {

		for (auto child : queue.front()->children)
			queue.push(child);

		auto it = std::find_if(list.begin(), list.end(),
			[&](std::weak_ptr<RenderObject> ptr) {
				return queue.front() == ptr.lock();
			});
		if (it != list.end())
			list.erase(it);

		queue.pop();
		};

	switch (renderType) {
	case WORLD: while (!queue.empty()) { removeFrom(worldList); }; break;
	case VIEW: while (!queue.empty()) { removeFrom(viewList); }; break;
	case SCREEN: while (!queue.empty()) { removeFrom(screenList); }; break;
	default: break;
	}

	newParent->NewChild(thisShared);
}

std::shared_ptr<RenderObject> RenderObject::CloneAsChildOf(const std::shared_ptr<RenderObject>& parent) const {
	auto copy = CloneSelf();
	if (!copy)
		throw std::runtime_error("failed to Clone self with groupName: " + name + ", and renderType: " + std::to_string(renderType));
	copy->CloneChildrenFrom(*this);
	parent->NewChild(copy);
	return newObject;
}

glm::mat4 RenderObject::GetModel() {
	std::stack < std::shared_ptr<RenderObject>> hierarchyStack;

	std::shared_ptr<RenderObject> selected = shared_from_this();
	while (selected) {
		hierarchyStack.push(selected);
		selected = selected->parent.lock();
	}

	MatrixStack thisModelStack;
	thisModelStack.Clear();
	thisModelStack.LoadIdentity();

	while (!hierarchyStack.empty()) {
		TransformModelStack(thisModelStack, hierarchyStack.top());
		hierarchyStack.pop();
	}

	return thisModelStack.Top();
}

void RenderObject::UpdateModelWithParent(glm::mat4 parentModel) {
	MatrixStack ms;
	ms.Clear();
	ms.LoadMatrix(parentModel);
	TransformModelStack(ms, shared_from_this());
	model = ms.Top();
	for (auto child : children) {
		child->UpdateModelWithParent(model);
	}
	isDirty = false;
}

std::shared_ptr<RenderObject> RenderObject::Clone() const {
	auto copy = CloneSelf();
	if (!copy)
		throw std::runtime_error("failed to Clone self with groupName: " + name + ", and renderType: " + std::to_string(renderType));
	copy->CloneChildrenFrom(*this);
	return copy;
}

void RenderObject::CloneChildrenFrom(const RenderObject& parentOfClonedChidren) {
	for (auto& child : parentOfClonedChidren.children) {
		children.push_back(child->Clone());
		children.back()->parent = shared_from_this();
	}
}

void RenderObject::AddHierarchyToList(RENDER_TYPE type, std::shared_ptr<RenderObject> obj) {
	std::queue<std::shared_ptr<RenderObject>> queue;
	queue.push(obj);

	auto addTo = [&](std::vector<std::weak_ptr<RenderObject>>& list) {
		for (auto child : queue.front()->children)
			queue.push(child);

		queue.front()->renderType = type;
		list.push_back(queue.front());

		queue.pop();
		};

	switch (type) {
	case WORLD:  while (!queue.empty()) { addTo(worldList); }; break;
	case VIEW: while (!queue.empty()) { addTo(viewList); }; break;
	case SCREEN: while (!queue.empty()) { addTo(screenList); }; break;
	default: break;
	}
}

void RenderObject::TransformModelStack(MatrixStack& modelStack, const std::shared_ptr<RenderObject>& obj) {

	float relativeX = 1, relativeY = 1, relativeOffsetX = 0, relativeOffsetY = 0;
	if (obj->renderType == RenderObject::SCREEN && obj->relativeTrl) {
		relativeOffsetX = relativeX = App::SCREEN_WIDTH / 2;
		relativeOffsetY = relativeY = App::SCREEN_HEIGHT / 2;
	}

	modelStack.Translate(obj->trl.x * relativeX + relativeOffsetX, obj->trl.y * relativeY + relativeOffsetY, obj->trl.z);
	modelStack.Rotate(obj->rot.x, 1, 0, 0);
	modelStack.Rotate(obj->rot.y, 0, 1, 0);
	modelStack.Rotate(obj->rot.z, 0, 0, 1);
	modelStack.Scale(obj->scl.x, obj->scl.y, obj->scl.z);

	if (obj->offsetTrl != vec3(0))
		modelStack.Translate(obj->offsetTrl.x, obj->offsetTrl.y, obj->offsetTrl.z);
	if (obj->offsetRot.x != 0)
		modelStack.Rotate(obj->offsetRot.x, 1, 0, 0);
	if (obj->offsetRot.y != 0)
		modelStack.Rotate(obj->offsetRot.y, 0, 1, 0);
	if (obj->offsetRot.z != 0)
		modelStack.Rotate(obj->offsetRot.z, 0, 0, 1);
	if (obj->offsetScl != vec3(1))
		modelStack.Scale(obj->offsetScl.x, obj->offsetScl.y, obj->offsetScl.z);
}

/********************************* MeshObject *********************************/

std::shared_ptr<MeshObject> MeshObject::Create(int geometryType, unsigned UILayer) {
	auto obj = std::make_shared<MeshObject>(geometryType, UILayer);
	setDefaultStat.Invoke(geometryType, obj);
	return obj;
}

std::shared_ptr<RenderObject> MeshObject::CloneSelf() const {
	auto obj = std::make_shared<MeshObject>(*this);
	obj->children.clear();
	obj->parent.reset();
	return obj;
}


/********************************* LightObject *********************************/

int LightObject::maxLight = 12;
std::vector<std::weak_ptr<LightObject>> LightObject::lightList;

std::shared_ptr<LightObject> LightObject::Create(int geometryType, unsigned UILayer) {
	if (lightList.size() > maxLight)
		return nullptr;

	auto obj = std::make_shared<LightObject>(geometryType, UILayer);
	lightList.push_back(obj);
	obj->lightIndex = lightList.size() - 1;
	setDefaultStat.Invoke(geometryType, obj);

	return obj;
}

LightObject::~LightObject() {
	bool firstEntry = true;
	for (unsigned i = 0; i < lightList.size(); i++) {

		if (firstEntry) {
			i = this->lightIndex + 1;
			firstEntry = false;
			if (i == lightList.size())
				break;
		}

		if (!lightList[i].expired()) {
			auto lightObj = lightList[i].lock();
			lightObj->lightIndex--;
		}
	}
};

std::shared_ptr<RenderObject> LightObject::CloneSelf() const {

	auto obj = std::make_shared<LightObject>(*this);
	lightList.push_back(obj);
	obj->lightIndex = lightList.size() - 1;
	obj->children.clear();
	obj->parent.reset();

	return obj;
}


/********************************* TextObject *********************************/

std::shared_ptr<TextObject> TextObject::Create(std::string id, std::string text, glm::vec3 color, int font, bool centerText, unsigned UILayer) {
	auto obj = std::make_shared<TextObject>(id, text, color, font, centerText, UILayer);
	setDefaultStat.Invoke(font, obj);
	return obj;
}

std::shared_ptr<RenderObject> TextObject::CloneSelf() const {
	auto obj = std::make_shared<TextObject>(*this);
	obj->children.clear();
	obj->parent.reset();
	return obj;
}
