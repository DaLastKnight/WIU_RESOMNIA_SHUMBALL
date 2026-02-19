#define _USE_MATH_DEFINES
#include <cmath>

#include "SceneRhythm.h"

//Include GLEW
#include <GL/glew.h>
//Include GLFW
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_inverse.hpp>

#include "Light.h"
#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "TextureLoader.h"
#include "MouseController.h"
#include "KeyboardController.h"
#include "AudioManager.h"
#include "DataManager.h"

#include "Console.h"
#include "Utils.h"

using GEO = GEOMETRY_TYPE;
using App = Application;
using RObj = RenderObject;
using Cam = FPCamera;

using glm::vec3;
using glm::mat4;
using std::string;


/*****************************************************************************************************************************************************************************************/
/************************************************************************************ scene functions ************************************************************************************/
/*****************************************************************************************************************************************************************************************/

SceneRhythm::SceneRhythm() {
}

SceneRhythm::~SceneRhythm() {
}

void SceneRhythm::Init() {
	BaseScene::Init();

	// directory init
	{
		AudioManager::GetInstance().SetDirectoryMUS("SceneRhythm/Music");
		AudioManager::GetInstance().SetDirectorySFX("SceneRhythm/SFX");
		TextureLoader::SetDirectory("SceneRhythm/Image");
		ModelLoader::SetDirectory("SceneRhythm/Model");
	}

	// audio init
	{
		// music init
		AudioManager::GetInstance().LoadMUS("A_CYBERS_WORLD_DELTARUNE_Chapter_2_Soundtrack_Toby_Fox.ogg", 166); 

		// sfx init
		AudioManager::GetInstance().LoadSFX(GOOFY_AHH_ASRIEL_STAR_SOUND, "sfx_asriel_star_drop.wav");

	}

	// atmosphere init
	{
		atmosphere.Set(vec3(0.05f, 0.07f, 0.1f), 0.05f, 0.000001f, 2, 20);
		UpdateAtmosphereUniform();
	}	

	// Init VBO here
	{
		for (int i = 0; i < static_cast<int>(TOTAL); ++i)
		{
			meshList[i] = nullptr;
		}
		meshList[AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTGA("color.tga"));
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTGA("skybox.tga"));
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTGA("Cascadia_Mono.tga"));
	}

	// init roots
	{
		worldRoot = std::make_shared<RObj>();
		worldRoot->renderType = RObj::WORLD;
		worldRoot->geometryType = GROUP;
		worldRoot->UpdateModel();

		viewRoot = std::make_shared<RObj>();
		viewRoot->renderType = RObj::VIEW;
		viewRoot->geometryType = GROUP;
		viewRoot->UpdateModel();

		screenRoot = std::make_shared<RObj>();
		screenRoot->renderType = RObj::SCREEN;
		screenRoot->geometryType = GROUP;
		screenRoot->UpdateModel();

		LightObject::maxLight = MAX_LIGHT;
		LightObject::lightList.reserve(MAX_LIGHT);

		RObj::worldList.reserve(50);
		RObj::viewList.reserve(10);
		RObj::screenList.reserve(10);
	}

	// init default stats
	{
		RObj::setDefaultStat.Subscribe(AXES, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT); 
			});
		RObj::setDefaultStat.Subscribe(GROUND, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);
			});
		RObj::setDefaultStat.Subscribe(SKYBOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT); 
			});
		RObj::setDefaultStat.Subscribe(LIGHT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON); 
			obj->offsetScl = vec3(0.05f);
			});
		RObj::setDefaultStat.Subscribe(GROUP, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);
			obj->offsetScl = vec3(0.15f);
			});
		RObj::setDefaultStat.Subscribe(FONT_CASCADIA_MONO, [](const std::shared_ptr<RObj>& obj) {
			});

	}

	auto& newObj = RObj::newObject;
	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(AXES));

		worldRoot->NewChild(MeshObject::Create(GROUND));

		worldRoot->NewChild(MeshObject::Create(SKYBOX));

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;

			worldRoot->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(0, 20, 0);
				newLightObj->name = "demo light";
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_POINT;
				lightProperties.color = vec3(1, 1, 1);
				lightProperties.power = 1;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.001f;
				lightProperties.kQ = 0.001f;
				UpdateLightUniform(newLightObj);
			}

			worldRoot->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(20, 5, 0);
				newLightObj->name = "demo light spot";
				newLightObj->initialDire = vec3(0, -1, 0); 
				newLightObj->rot = vec3(45, 45, 0);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_SPOT;
				lightProperties.color = vec3(1, 0.824f, 0.11f);
				lightProperties.power = 1;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.005f;
				lightProperties.kQ = 0.01f;
				// spot light variables 
				lightProperties.cosCutoff = 31.f;
				lightProperties.cosInner = 29.f;
				UpdateLightUniform(newLightObj);
			}
		}
	}

	// view space init
	{
		
	}

	// screen space init
	{


		// debug text
		InitDebugText(FONT_CASCADIA_MONO); 
	}

	/************************ bellow for external class inits ************************/
	{
		// camera init
		camera.Init(glm::vec3(1, 2, -1), glm::vec3(-1, -1, 1));
		camera.Set(FPCamera::MODE::FREE);

		// player init
		player.Init(worldRoot, GROUP, vec3(0, 2, 0));
	}

	RObj::newObject.reset();
}

void SceneRhythm::Update(double dt) {
	BaseScene::Update(dt);
	ClearDebugText();

	// fps calculation
	{
		static float timer = 0;
		static int frameCount = 0;
		static float avgFps = 0;
		const float fpsUpdateTime = 0.5f;
		timer += dt;
		frameCount++;
		if (timer >= fpsUpdateTime) {
			avgFps = frameCount / timer;
			timer = 0;
			frameCount = 0;
		}
		AddDebugText("avg fps / 0.5s: " + std::to_string(avgFps));
	}

	if (dt > 0.1f) {
		dt = 0.1f;
	}

	auto& lightList = LightObject::lightList;
	auto& worldList = RObj::worldList;
	auto& viewList = RObj::viewList;
	auto& screenList = RObj::screenList;

	
	HandleKeyPress();

	// player
	{
		// update position and camera bobbing
		if (player.allowControl)
			player.UpdatePositionWithCamera(dt, camera);
		else {
			Cam tempCamera = camera;
			player.UpdatePositionWithCamera(dt, tempCamera);
		}
		
		player.SyncRender();
	}

	// camera
	camera.Update(dt);

	if (debug) {
		AddDebugText("camera.basePosition: " + VecToString(camera.basePosition)); 
		AddDebugText("worldRoot.model.trl: " + VecToString(getPosFromModel(worldRoot->model)));
		AddDebugText("viewRoot.trl: " + VecToString(getPosFromModel(viewRoot->model)));
		AddDebugText("screenRoot.trl: " + VecToString(getPosFromModel(screenRoot->model)));
	}

	// world render objects
	for (unsigned i = 0; i < worldList.size(); ) {
		if (worldList[i].expired()) {
			worldList.erase(worldList.begin() + i);
			continue;
		}
		auto obj = worldList[i].lock();

		switch (obj->geometryType) {
		case AXES:
		case GROUP:
			obj->allowRender = debug;
			break;

		case SKYBOX:
			obj->trl = camera.GetFinalPosition();
			break;

		default:
			break;
		}

		
		if (obj->name == "demo light spot") {
			obj->offsetRot.y += 45 * dt;
			obj->isDirty = true; 
		} 

		if (debug) {

		}

		obj->UpdateModel(); 
		i++;
	}

	// view render objects
	for (unsigned i = 0; i < viewList.size(); ) {
		if (viewList[i].expired()) {
			viewList.erase(viewList.begin() + i);
			continue;
		}
		auto obj = viewList[i].lock();



		if (debug) {

		}

		obj->UpdateModel();
		i++;
	}

	// screen render objects
	for (unsigned i = 0; i < screenList.size(); ) {
		if (screenList[i].expired()) {
			screenList.erase(screenList.begin() + i);
			continue;
		}
		auto obj = screenList[i].lock();


		if (debug) {

		}

		obj->UpdateModel();
		obj->trl.z = 0;
		obj->scl.z = 1;
		i++;
	}

	// light update
	for (unsigned i = 0; i < lightList.size(); ) {
		if (lightList[i].expired()) {
			lightList.erase(lightList.begin() + i);
			continue;
		}
		auto obj = lightList[i].lock();
		obj->allowRender = debug;
		Light& properties = obj->lightProperties;



		if (debug) {

		}

		obj->UpdateModel();

		// update light's position and possibly rotation with model
		if (obj->renderType == RObj::VIEW || obj->renderType == RObj::WORLD) {
			mat4 lightModel = obj->model;

			// find world space model
			if (obj->renderType == RObj::VIEW) {
				mat4 inversedView = glm::inverse(viewStack.Top());
				lightModel = inversedView * lightModel; // world_space_model = view_space_model / view_mat, / view_mat == inverseView
			}
			properties.position = getPosFromModel(lightModel);
			if (properties.type == Light::LIGHT_SPOT)
				properties.spotDirection = rotateScaleWithModel(lightModel, obj->initialDire);
		}
		else {
			properties.power = 0;
		}

		UpdateLightUniform(obj);
		i++;
	}

}


void SceneRhythm::Render() {
	BaseScene::Render();
	
	// render scene
	struct ListInfo {
		std::shared_ptr<RObj> obj;
		mat4 model;
		float depth;
		ListInfo(std::shared_ptr<RObj> obj, mat4 model, float depth)
			: obj(obj), model(model), depth(depth) {}
	};
	std::vector<ListInfo> transparencyList;
	transparencyList.reserve(40);

	auto insert2TransparencyList = [&](const std::shared_ptr<RObj>& obj, const mat4& model, float depth) {

		unsigned insertPosition = 0;

		for (auto& info : transparencyList) {
			if (info.depth > depth)
				insertPosition++;
			else
				break;
		}

		transparencyList.emplace(transparencyList.begin() + insertPosition, obj, model, depth);
		};
	auto renderTransparencyList = [&]() {
		for (auto& info : transparencyList) {
			modelStack.PushMatrix();
			modelStack.LoadMatrix(info.model);
			RenderObj(info.obj);
			modelStack.PopMatrix();
		}
		};

	auto renderObjectList = [&](const std::vector<std::weak_ptr<RObj>>& list, bool ignoreTransparency = false) {
		for (auto& obj_wptr : list) {
			auto obj = obj_wptr.lock();
			modelStack.PushMatrix();
			modelStack.LoadMatrix(obj->model);

			if (obj->hasTransparency && !ignoreTransparency) {
				vec3 obj_worldPos = vec3(modelStack.Top()[3]);
				vec3 obj2CameraPos = camera.GetFinalPosition() - obj_worldPos;
				float depthSqr = obj2CameraPos.x * obj2CameraPos.x + obj2CameraPos.y * obj2CameraPos.y + obj2CameraPos.z * obj2CameraPos.z;
				insert2TransparencyList(obj, modelStack.Top(), depthSqr);
			}
			else
				RenderObj(obj);

			modelStack.PopMatrix();
		}
		};


	renderObjectList(RObj::worldList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	renderTransparencyList();
	transparencyList.clear();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);


	viewStack.PushMatrix();
	viewStack.LoadIdentity();

	renderObjectList(RObj::viewList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	renderTransparencyList();
	transparencyList.clear();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);


	glDisable(GL_DEPTH_TEST);

	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	RObj::SortScreenList();
	renderObjectList(RObj::screenList, true);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.Clear();

	glEnable(GL_DEPTH_TEST);

}

void SceneRhythm::Exit() {
	BaseScene::Exit();


}

void SceneRhythm::HandleKeyPress() {
	BaseScene::HandleKeyPress();

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
		debug = !debug;

		if (debug) {
			camera.Set(Cam::MODE::FREE);
			player.allowControl = false;
		}
		else {
			camera.Set(Cam::MODE::FIRST_PERSON);
			player.allowControl = true;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_LEFT_ALT)) {
		static auto prevMode = Cam::MODE::FREE;
		auto& cameraMode = camera.GetCurrentMode();

		if (cameraMode != Cam::MODE::PAUSE) {
			prevMode = cameraMode;
			camera.Set(Cam::MODE::PAUSE);
		}
		else {
			camera.Set(prevMode);
		}
	}

	// player controls
	if (player.allowControl) {

		// movement
		{
			auto playerGroup = player.renderGroup.lock();
			vec3 rightDire = glm::normalize(glm::cross(player.direction, vec3(0, 1, 0)));
			vec3 finalTrlChange = vec3(0);
			float cameraPsiRotation = 2.f;

			player.sprinting = false;
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
				player.sprinting = true;
			}

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W)) {
				finalTrlChange += player.direction;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S)) {
				finalTrlChange -= player.direction;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A)) {
				finalTrlChange -= rightDire;
				camera.psi -= cameraPsiRotation;
				if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W) || KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
					camera.psi -= cameraPsiRotation * 0.5f;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D)) {
				finalTrlChange += rightDire;
				camera.psi += cameraPsiRotation;
				if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W) || KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
					camera.psi += cameraPsiRotation * 0.5f;
			}

			player.velocity = finalTrlChange;
		}

		// action
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::LMB)) {
			AudioManager::GetInstance().PlayMUS(0, 1);
		}
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB)) {
			AudioManager::GetInstance().PlaySFX(GOOFY_AHH_ASRIEL_STAR_SOUND);
		}
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::MMB)) {
			if (AudioManager::GetInstance().PlayingMUS())
				AudioManager::GetInstance().PauseMUS();
			else
				AudioManager::GetInstance().ResumeMUS();
		}
		if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) > 0) {
			AudioManager::GetInstance().SetMUSPosition(AudioManager::GetInstance().GetMUSPosition() + 1);
		}
		if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) < 0) {
			AudioManager::GetInstance().SetMUSPosition(AudioManager::GetInstance().GetMUSPosition() - 1);
		}
	}
}


/*********************************************************************************************************************************************************************************/
/************************************************************************************ helpers ************************************************************************************/
/*********************************************************************************************************************************************************************************/


void SceneRhythm::RenderObj(const std::shared_ptr<RObj> obj) {

	if (!obj->allowRender)
		return;

	bool enableLight = true;
	if (obj->material.type == Material::NO_LIGHT || obj->renderType == RObj::SCREEN)
		enableLight = false;

	Material meshMaterial = meshList[obj->geometryType]->material;
	if (obj->material.type != Material::MESH_MATERIAL) {
		meshList[obj->geometryType]->material = obj->material;
	}

	if (auto textObj = std::dynamic_pointer_cast<TextObject>(obj)) {
		modelStack.PushMatrix();

		const auto& text = textObj->text;
		const auto& mesh = meshList[obj->geometryType];

		glDisable(GL_CULL_FACE);

		glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
		glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &textObj->color.r);
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);

		// offset
		float spacing = FontSpacing(static_cast<GEOMETRY_TYPE>(textObj->geometryType));
		if (textObj->centerText)
			modelStack.Translate(text.size() * spacing / -2.f + spacing / 2, 0, 0);

		for (unsigned i = 0; i < text.length(); ++i)
		{
			glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(i * spacing, 0, 0));
			glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
			glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

			mesh->Render((unsigned)text[i] * 6, 6);
		}

		if (cullFaceActive)
			glEnable(GL_CULL_FACE);

		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

		modelStack.PopMatrix();
	}
	else {
		RenderMesh(static_cast<GEOMETRY_TYPE>(obj->geometryType), enableLight);
	}

	meshList[obj->geometryType]->material = meshMaterial;

}



void SceneRhythm::RenderMesh(GEOMETRY_TYPE type, bool enableLight) {

	Mesh* mesh = meshList[static_cast<int>(type)];
	glm::mat4 MVP, modelView, modelView_inverse_transpose;
	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
	modelView = viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, glm::value_ptr(modelView));

	if (enableLight)
	{
		glUniform1i(m_parameters[U_LIGHT_ENABLED], 1);
		modelView_inverse_transpose = glm::inverseTranspose(modelView);
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, glm::value_ptr(modelView_inverse_transpose));

		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{
		glUniform1i(m_parameters[U_LIGHT_ENABLED], 0);
	}

	if (mesh->textureID > 0)
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}

	mesh->Render();

	if (mesh->textureID > 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SceneRhythm::InitDebugText(GEOMETRY_TYPE font) {
	auto& newObj = RObj::newObject;
	for (int i = 0; i < 10; i++) {
		screenRoot->NewChild(TextObject::Create("_debugtxt_" + std::to_string(i), "", vec3(0, 1, 0), font, false, 99));
		newObj->relativeTrl = true;
		newObj->trl = vec3(-0.98f, 0.95f - i * 0.05f, 0);
		newObj->scl = vec3(30, 30, 1);
		debugTextList.push_back(newObj);
	}
}

bool SceneRhythm::AddDebugText(const std::string& text, int index) {

	if (index < 0) {
		for (auto& obj_weak : debugTextList) {
			auto textObj = std::dynamic_pointer_cast<TextObject>(obj_weak.lock());

			if (textObj->text == "") {
				textObj->text = text;
				return true;
			}
		}
		return false;
	}

	index = Clamp(index, 0, 9);
	std::dynamic_pointer_cast<TextObject>(debugTextList[index].lock())->text = text;

	return true;
}

void SceneRhythm::ClearDebugText() {
	for (auto& obj_weak : debugTextList)
		std::dynamic_pointer_cast<TextObject>(obj_weak.lock())->text = "";
}
