#define _USE_MATH_DEFINES
#include <cmath>

#include "SceneBowling.h"

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


/* notes:
* if you are unsure if a certain function does something, hover over it and see what it does (i added description for most of the functions you might need help to know what it does)
*/


/*****************************************************************************************************************************************************************************************/
/************************************************************************************ scene functions ************************************************************************************/
/*****************************************************************************************************************************************************************************************/

SceneBowling::SceneBowling() {
}

SceneBowling::~SceneBowling() {
}

void SceneBowling::Init() {
	BaseScene::Init();

	// directory init
	{
		AudioManager::GetInstance().SetDirectoryMUS("SceneBowling/Music");
		AudioManager::GetInstance().SetDirectorySFX("SceneBowling/SFX");
		TextureLoader::SetDirectory("SceneBowling/Image");
		ModelLoader::SetDirectory("SceneBowling/Model");
	}

	// audio init
	{
		// music init
		AudioManager::GetInstance().LoadMUS("Wheel_Chill.ogg", 57.7555); // you need to input the total duration of the music in seconds as a double, sdl mixer cannot get the duration itself

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
		for (int i = 0; i < static_cast<int>(GEO::TOTAL); ++i)
		{
			meshList[i] = nullptr;
		}
		meshList[GEO::AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GEO::GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTGA("color.tga"));
		meshList[GEO::SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTGA("skybox.tga"));
		meshList[GEO::LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GEO::GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));

		meshList[GEO::FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(GEO::FONT_CASCADIA_MONO), TextureLoader::LoadTGA("Cascadia_Mono.tga"));

		meshList[GEO::FLASHLIGHT] = MeshBuilder::GenerateOBJMTL("flashlight", "flashlight.obj", "flashlight.mtl", TextureLoader::LoadTGA("flashlight_texture.tga"));
		meshList[GEO::Bowling_Pin] = MeshBuilder::GenerateOBJMTL("Bowling_Pin", "Bowling_Pin.obj", "Bowling_Pin.mtl", TextureLoader::LoadTGA("Bowling_Pin.tga"));
		meshList[GEO::Bowling_Ball] = MeshBuilder::GenerateOBJMTL("Bowling_Ball", "Bowling_Ball.obj", "Bowling_Ball.mtl", TextureLoader::LoadTGA("Bowling_Ball.tga"));

		meshList[GEO::UI_TEST] = MeshBuilder::GenerateQuad("ui test", vec3(1), 1, 1, TextureLoader::LoadTGA("color.tga"));
		meshList[GEO::UI_TEST_2] = MeshBuilder::GenerateQuad("ui test 2", vec3(1), 1, 1, TextureLoader::LoadTGA("color.tga"));
	}

	// init roots
	{
		worldRoot = std::make_shared<RObj>();
		worldRoot->renderType = RObj::WORLD;
		worldRoot->geometryType = GEO::GROUP;
		worldRoot->UpdateModel();

		viewRoot = std::make_shared<RObj>();
		viewRoot->renderType = RObj::VIEW;
		viewRoot->geometryType = GEO::GROUP;
		viewRoot->UpdateModel();

		screenRoot = std::make_shared<RObj>();
		screenRoot->renderType = RObj::SCREEN;
		screenRoot->geometryType = GEO::GROUP;
		screenRoot->UpdateModel();

		LightObject::maxLight = MAX_LIGHT;
		LightObject::lightList.reserve(MAX_LIGHT);

		RObj::worldList.reserve(50);
		RObj::viewList.reserve(10);
		RObj::screenList.reserve(10);
	}

	// init default stats
	{
		RObj::setDefaultStat.Subscribe(GEO::AXES, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT); // does not get affected by light, always bright (fog does not work on objects not affected by light)
			});
		RObj::setDefaultStat.Subscribe(GEO::GROUND, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);
			});
		RObj::setDefaultStat.Subscribe(GEO::SKYBOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT); // affected by light, tho the material is set in a way so that it is always bright, just like NO_LIGHT (this makes sure fog can still be casted on it while be bright at times without fog)
			});
		RObj::setDefaultStat.Subscribe(GEO::LIGHT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON); // bright when shinned with light directly and and still be rather bright when not shinned
			obj->offsetScl = vec3(0.05f);
			});
		RObj::setDefaultStat.Subscribe(GEO::GROUP, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);
			obj->offsetScl = vec3(0.15f);
			});
		RObj::setDefaultStat.Subscribe(GEO::FONT_CASCADIA_MONO, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(GEO::FLASHLIGHT, [](const std::shared_ptr<RObj>& obj) {
			});


		RObj::setDefaultStat.Subscribe(GEO::Bowling_Pin, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(GEO::Bowling_Ball, [](const std::shared_ptr<RObj>& obj) {
			});


		RObj::setDefaultStat.Subscribe(GEO::UI_TEST, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GEO::UI_TEST_2, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
	}

	auto& newObj = RObj::newObject;
	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(GEO::AXES));

		worldRoot->NewChild(MeshObject::Create(GEO::GROUND));

		worldRoot->NewChild(MeshObject::Create(GEO::SKYBOX));

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;

			worldRoot->NewChild(LightObject::Create(GEO::LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj); // casting the obj to its actual type to acess variables only in its actual type
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

			worldRoot->NewChild(LightObject::Create(GEO::LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(20, 5, 0);
				newLightObj->name = "demo light spot";
				newLightObj->initialDire = vec3(0, -1, 0); // must have this to define the initial spotDirection for spot light, default vec3(0, -1, 0)
				newLightObj->rot = vec3(45, 45, 0);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_SPOT;
				lightProperties.color = vec3(1, 0.824f, 0.11f); // orange flame color
				lightProperties.power = 1;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.005f;
				lightProperties.kQ = 0.01f;
				// spot light variables (yes, these are the only 2 you need to change manually)
				lightProperties.cosCutoff = 31.f;
				lightProperties.cosInner = 29.f;
				UpdateLightUniform(newLightObj);
			}
		}

		//bowling pins 
		{
		
			worldRoot->NewChild(MeshObject::Create(GEO::Bowling_Pin));
			newObj->trl = glm::vec3(0.35f, 0.0f, 0.5f);
		
		}

		//Bowling ball
		{
			worldRoot->NewChild(MeshObject::Create(GEO::Bowling_Ball));
			newObj->trl = glm::vec3(-0.35f, 0.0f, -0.5f);
			newObj->scl = glm::vec3(0.35f, 0.35f, 0.35f);
			//newObj->rot = glm::vec3(-0.35f, 0.0f, -0.5f);
		}
	}

	// view space init
	{
		viewRoot->NewChild(MeshObject::Create(GEO::FLASHLIGHT));
		newObj->trl = glm::vec3(-0.35f, -0.2f, -0.5f);
	}

	// screen space init
	{
		screenRoot->NewChild(MeshObject::Create(GEO::UI_TEST, 1));  // create with 1 as UILayer, default 0
		newObj->trl = vec3(-0.8f, -0.8f, 0); // give any number for z, itll be force set to 0 in the loop
		newObj->scl = vec3(80, 80, 1); // give any number for z, itll be force set to 1 in the loop


		//screenRoot->NewChild(MeshObject::Create(GEO::UI_TEST_2));
		//newObj->trl = vec3(-0.85f, -0.85f, 0);
		//newObj->scl = vec3(80, 80, 1);


		// debug text
		InitDebugText(GEO::FONT_CASCADIA_MONO); // if you want another font for debug text, just change it to another font, tho dont call this in Update(), itll break
	}

	/************************ bellow for external class inits ************************/
	{
		// camera init
		camera.Init(glm::vec3(1, 2, -1), glm::vec3(-1, -1, 1));
		camera.Set(FPCamera::MODE::FREE);

		// player init
		player.Init(worldRoot, GEO::GROUP, vec3(0, 2, 0));
	}

	RObj::newObject.reset();
}

void SceneBowling::Update(double dt) {
	BaseScene::Update(dt);

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

	// if you ever felt that you need dt inside HandleKeyPress(), that means you are doing smt wro- i mean, you need to use a variable to pass the info and commit changes in Update instead, HandleKeyPress() should not have those kinda logic inside it
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
		// make sure the player's render group is updated to be the same as player's actual position
		player.SyncRender();
	}

	// camera
	camera.Update(dt); // this must be right after player's block of code to make sure it is sync

	// yah you can do this to add text, but this must be called every frame since it gets refreshed every frame
	// you can call AddDebugText() at anywhere after calling BaseScene::Update(); and before calling renderObjectList(RObj::screenList, true); and itll work
	if (debug) {
		AddDebugText("camera.basePosition: " + VecToString(camera.basePosition)); // VecToString supports vec2, vec3 and vec4 (idfk why i didt that but why not ig)
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
		case GEO::AXES:
		case GEO::GROUP:
			obj->allowRender = debug;
			break;

		case GEO::SKYBOX:
			obj->trl = camera.GetFinalPosition();
			break;

		default:
			break;
		}

		// btw. this code here visual does nothing, if you turn un debug and get close to to spot light and see it, youll realise its rotating perpendicularly to the light direction
		if (obj->name == "demo light spot") {
			obj->offsetRot.y += 45 * dt;
			obj->isDirty = true; // UpdateModel() cannot detect changes in offsets, so you need to manually set isDirty to true
		} // tho normally you wont need to touch offsets in Update() at all since you normally will have a group obj that is parented to this

		if (debug) {

		}

		obj->UpdateModel(); // detects changes in trl, rot and scl automatically to update its hierarchy's model
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


void SceneBowling::Render() {
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

void SceneBowling::Exit() {
	BaseScene::Exit();


}

void SceneBowling::HandleKeyPress() {
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

