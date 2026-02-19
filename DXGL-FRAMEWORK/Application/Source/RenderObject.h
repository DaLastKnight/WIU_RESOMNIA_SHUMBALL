#ifndef RRENDER_OBJECT_H
#define RRENDER_OBJECT_H

#include <string>
#include <memory>
#include <vector>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "MatrixStack.h"
#include "Material.h"
#include "Event.h"
#include "Light.h"


inline glm::vec3 getPosFromModel(const glm::mat4& model) {
	return glm::vec3(model[3]);
}
inline glm::vec3 rotateScaleWithModel(const glm::mat4& model, const glm::vec3 dire) {
	return glm::vec3(model * glm::vec4(dire, 0.0f));
}

class RenderObject : public std::enable_shared_from_this<RenderObject> {
public:

	glm::mat4 model;

	glm::vec3 trl = glm::vec3(0, 0, 0);
	glm::vec3 rot = glm::vec3(0, 0, 0);
	glm::vec3 scl = glm::vec3(1, 1, 1);
	glm::vec3 offsetTrl = glm::vec3(0, 0, 0);
	glm::vec3 offsetRot = glm::vec3(0, 0, 0);
	glm::vec3 offsetScl = glm::vec3(1, 1, 1);

	int UILayer = 0; // ranges from 0 to MAX_UI_LAYERS, anything else will be clamped in the calculation

	int geometryType;
	Material material;

	bool hasTransparency = false;
	bool allowRender = true;
	bool relativeTrl = false; // only affect screen render, trl will be from -1 to 1 in relative distance to center and side of the screen, instead of in px

	std::string name = "";

	enum RENDER_TYPE {
		WORLD,
		SCREEN,
		VIEW,

		TOTAL_RENDER_TYPE
	};

	RENDER_TYPE renderType;

	std::weak_ptr<RenderObject> parent;
	std::vector<std::shared_ptr<RenderObject>> children;

	static std::vector<std::weak_ptr<RenderObject>> worldList;
	static std::vector<std::weak_ptr<RenderObject>> viewList;
	static std::vector<std::weak_ptr<RenderObject>> screenList;

	// will be updated from the use of NewChild(), SwapParentTo() and CloneAsChildOf()
	// remember to reset this after using any of the above functions to avoid unwanted ownership
	static std::shared_ptr<RenderObject> newObject;

	// Subscribe() to a lambda and itll be Invoke() when creating a object with the same key
	static EventPack<int, void, const std::shared_ptr<RenderObject>&> setDefaultStat;

	bool isDirty = true;

	static void SortScreenList();

	void UpdateModel();

	void Destroy();
	void NewChild(std::shared_ptr<RenderObject> child);

	void SwapParentTo(const std::shared_ptr<RenderObject>& newParent);
	std::shared_ptr<RenderObject> CloneAsChildOf(const std::shared_ptr<RenderObject>& parent) const;

	virtual ~RenderObject() = default;
	RenderObject(int geometryType, RENDER_TYPE renderType, unsigned UILayer = 0)
		: geometryType(geometryType), renderType(renderType), UILayer(UILayer) {
	}
	RenderObject(int geometryType, unsigned UILayer = 0)
		: geometryType(geometryType), renderType(WORLD), UILayer(UILayer) {
	}
	RenderObject()
		: geometryType(static_cast<int>(0)), renderType(WORLD) {
	};

protected:

	glm::vec3 prevTrl = trl;
	glm::vec3 prevRot = rot;
	glm::vec3 prevScl = scl;

	static constexpr unsigned MAX_UI_LAYERS = 100;

	glm::mat4 GetModel();
	void UpdateModelWithParent(glm::mat4 parentModel);

	std::shared_ptr<RenderObject> Clone() const;
	virtual std::shared_ptr<RenderObject> CloneSelf() const {
		return nullptr;
	}
	void CloneChildrenFrom(const RenderObject& parentOfClonedChidren);

	void AddHierarchyToList(RENDER_TYPE type, std::shared_ptr<RenderObject> obj);

	void TransformModelStack(MatrixStack& modelStack, const std::shared_ptr<RenderObject>& obj);

	
};


class MeshObject : public RenderObject {
public:

	static std::shared_ptr<MeshObject> Create(int geometryType, unsigned UILayer = 0);

	~MeshObject() = default;
	MeshObject(int geometryType, unsigned UILayer = 0)
		: RenderObject(geometryType, UILayer) {
	}

protected:

	std::shared_ptr<RenderObject> CloneSelf() const override;
};


struct LightObject : public RenderObject {
	Light lightProperties;
	unsigned lightIndex;
	glm::vec3 initialDire = glm::vec3(0, -1, 0); // for spot light, used to calculate the actual direction using the model of the light

	static int maxLight;
	static std::vector<std::weak_ptr<LightObject>> lightList;

	static std::shared_ptr<LightObject> Create(int geometryType, unsigned UILayer = 0);

	~LightObject();
	LightObject(int geometryType, unsigned UILayer = 0)
		: RenderObject(geometryType, UILayer) {
	}

protected:

	std::shared_ptr<RenderObject> CloneSelf() const override;
};


class TextObject : public RenderObject {
public:

	std::string text;
	glm::vec3 color;
	bool centerText = false;

	static std::shared_ptr<TextObject> Create(std::string name, std::string text, glm::vec3 color, int font, bool centerText = false, unsigned UILayer = 0);

	~TextObject() = default;
	TextObject(std::string name, std::string text, glm::vec3 color, int font, bool centerText = false, unsigned UILayer = 0)
		: text(text), color(color), centerText(centerText), RenderObject(font, UILayer) {
		hasTransparency = true;
		this->name = name;
	}

protected:

	std::shared_ptr<RenderObject> CloneSelf() const override;
};

#endif