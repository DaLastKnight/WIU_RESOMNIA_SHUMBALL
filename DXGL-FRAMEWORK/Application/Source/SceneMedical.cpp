#define _USE_MATH_DEFINES
#include <cmath>
#include <stdlib.h>

#include "SceneMedical.h"

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
#include "DialogueManager.h"

#include "Console.h"
#include "Utils.h"

using App = Application;
using RObj = RenderObject;
using Cam = FPCamera;
using PEvent = PhysicsEventListener::PhysicsEvent;

using glm::vec3;
using glm::mat4;
using std::string;


/* notes:
* if you are unsure if a certain function does something, hover over it and see what it does (i added description for most of the functions you might need help to know what it does)
*/


/*****************************************************************************************************************************************************************************************/
/************************************************************************************ scene functions ************************************************************************************/
/*****************************************************************************************************************************************************************************************/

SceneMedical::SceneMedical() {
}

SceneMedical::~SceneMedical() {
}


void SceneMedical::Init() {

	// set physics world settings
	auto& worldSettings = PhysicsManager::GetInstance().GetWorldSettingsObject();
	worldSettings.gravity = rp3d::Vector3(0, 0, 0);

	BaseScene::Init();

	{
		if (ALLOW_PHYSICS_DEBUG) {
			PhysicsManager::GetInstance().SetUpLogger("SceneDemo");
			PhysicsManager::GetInstance().SeteDebugRendering(true);
			PhysicsManager::GetInstance().SetDebugRenderItems(true, false, true, false, false);
		}
	}

	// directory init
	{
		AudioManager::GetInstance().SetDirectoryMUS("SceneMedical/Music");
		AudioManager::GetInstance().SetDirectorySFX("SceneMedical/SFX");
		TextureLoader::SetDirectory("SceneMedical/Image");
		ModelLoader::SetDirectory("SceneMedical/Model");
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
		for (int i = 0; i < static_cast<int>(TOTAL); ++i)
		{
			meshList[i] = nullptr;
		}
		meshList[AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTexture("color.tga"));
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTexture("skybox.tga"));
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));

		meshList[FLASHLIGHT] = MeshBuilder::GenerateOBJMTL("flashlight", "flashlight.obj", "flashlight.mtl", TextureLoader::LoadTexture("flashlight_texture.tga"));

		meshList[UI_TEST] = MeshBuilder::GenerateQuad("ui test", vec3(1), 1, 1, TextureLoader::LoadTexture("color.tga"));
		meshList[UI_TEST_2] = MeshBuilder::GenerateQuad("ui test 2", vec3(1), 1, 1, TextureLoader::LoadTexture("color.tga"));

		meshList[PNG_TEST] = MeshBuilder::GenerateQuad("png test", vec3(1), 1, 1, TextureLoader::LoadTexture("NYP.png"));

		meshList[ENV_SKYBOX] = MeshBuilder::GenerateSkybox("Map Boundary", TextureLoader::LoadTexture("internal_body.png"));
		meshList[ENV_SPHERE_MODEL] = MeshBuilder::GenerateSphere("Map Sphered Environment", vec3(1));
		meshList[ENV_SPHERE_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_BLOCK_MODEL] = MeshBuilder::GenerateCube("Map Block Environment", vec3(1), 1);
		meshList[ENV_BLOCK_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_STRING_MODEL] = MeshBuilder::GenerateCylinder("Map String Environment", vec3(1), 360, 0.5f, 100);
		meshList[ENV_STRING_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_LIQUID_MODEL] = MeshBuilder::GenerateCylinder("Map Liquid Environment", vec3(1), 360, 0.5f, 100);
		meshList[ENV_LIQUID_MODEL]->textureID = TextureLoader::LoadTexture("red_liquid.png");
		meshList[ENV_LIQUID_FLAT_MODEL] = MeshBuilder::GenerateQuad("Map Liquid Flat Environment", vec3(1), 1, 1, TextureLoader::LoadTexture("red_liquid.png"));
		meshList[BACTERIA_MODEL] = MeshBuilder::GenerateOBJMTL("bacteria", "bacteria.obj", "bacteria.mtl", TextureLoader::LoadTexture("bacteria_skin.png"));
		meshList[VIRUS_MODEL] = MeshBuilder::GenerateOBJMTL("virus", "bacteria.obj", "bacteria.mtl", TextureLoader::LoadTexture("virus_skin.png"));
		meshList[NANOBOT_MODEL] = MeshBuilder::GenerateOBJMTL("nanobot", "nanobot.obj", "nanobot.mtl", TextureLoader::LoadTexture("nanobot_skin.png"));

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

		RObj::worldList.reserve(200);
		RObj::viewList.reserve(10);
		RObj::screenList.reserve(10);
	}

	// init default stats
	{
		RObj::setDefaultStat.Subscribe(AXES, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT); // does not get affected by light, always bright (fog does not work on objects not affected by light)
			});
		RObj::setDefaultStat.Subscribe(GROUND, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);
			});
		RObj::setDefaultStat.Subscribe(SKYBOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT); // affected by light, tho the material is set in a way so that it is always bright, just like NO_LIGHT (this makes sure fog can still be casted on it while be bright at times without fog)
			});
		RObj::setDefaultStat.Subscribe(LIGHT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON); // bright when shinned with light directly and and still be rather bright when not shinned
			obj->offsetScl = vec3(0.05f);
			});
		RObj::setDefaultStat.Subscribe(GROUP, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);
			obj->offsetScl = vec3(0.15f);
			});
		RObj::setDefaultStat.Subscribe(FONT_CASCADIA_MONO, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(FLASHLIGHT, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(UI_TEST, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(UI_TEST_2, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(PNG_TEST, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(ENV_SKYBOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT); 
			});
		RObj::setDefaultStat.Subscribe(ENV_SPHERE_MODEL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});
		RObj::setDefaultStat.Subscribe(ENV_BLOCK_MODEL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});
		RObj::setDefaultStat.Subscribe(ENV_STRING_MODEL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});
		RObj::setDefaultStat.Subscribe(ENV_LIQUID_MODEL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});
		RObj::setDefaultStat.Subscribe(ENV_LIQUID_FLAT_MODEL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(BACTERIA_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(VIRUS_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(NANOBOT_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});
	}

	auto& newObj = RObj::newObject;
	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(AXES));

		worldRoot->NewChild(MeshObject::Create(SKYBOX));
		
		{
			worldRoot->NewChild(MeshObject::Create(ENV_SKYBOX));
			newObj->scl = vec3(0.1f, 0.1f, 0.1f);
		}

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;

			worldRoot->NewChild(LightObject::Create(LIGHT));
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

			worldRoot->NewChild(LightObject::Create(LIGHT));
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

		// Temporary Placeholder for Bacteria Model Spawning
		float bactRandTrlX[40] = { 0 };
		float bactRandTrlY[40] = { 0 };
		float bactRandTrlZ[40] = { 0 };

		for (int i = 0; i < 40; i++)
		{
			bactRandTrlX[i] = rand() % 81 - 40;
			bactRandTrlY[i] = rand() % 41 - 20;
			bactRandTrlZ[i] = rand() % 81 - 40;

			worldRoot->NewChild(MeshObject::Create(BACTERIA_MODEL));
			newObj->trl = glm::vec3(bactRandTrlX[i], bactRandTrlY[i], bactRandTrlZ[i]);
			newObj->scl = glm::vec3(0.5f, 0.5f, 0.5f);
		}

		// Temporary Placeholder for Virus Model Spawning
		float virusRandTrlX[20] = { 0 };
		float virusRandTrlY[20] = { 0 };
		float virusRandTrlZ[20] = { 0 };

		for (int i = 0; i < 20; i++)
		{
			virusRandTrlX[i] = rand() % 81 - 40;
			virusRandTrlY[i] = rand() % 41 - 20;
			virusRandTrlZ[i] = rand() % 81 - 40;

			worldRoot->NewChild(MeshObject::Create(VIRUS_MODEL));
			newObj->trl = glm::vec3(virusRandTrlX[i], virusRandTrlY[i], virusRandTrlZ[i]);
			newObj->scl = glm::vec3(0.5f, 0.5f, 0.5f);
		}

		worldRoot->NewChild(MeshObject::Create(ENV_SPHERE_MODEL));
		newObj->trl = glm::vec3(-50, -50, 50);
		newObj->scl = glm::vec3(20, 20, 20);
		worldRoot->NewChild(MeshObject::Create(ENV_SPHERE_MODEL));
		newObj->trl = glm::vec3(50, 50, -50);
		newObj->scl = glm::vec3(30, 30, 30);
		worldRoot->NewChild(MeshObject::Create(ENV_SPHERE_MODEL));
		newObj->trl = glm::vec3(50, -50, -50);
		newObj->scl = glm::vec3(30, 30, 30);
		worldRoot->NewChild(MeshObject::Create(ENV_SPHERE_MODEL));
		newObj->trl = glm::vec3(-50, 50, 50);
		newObj->scl = glm::vec3(20, 20, 20);
		worldRoot->NewChild(MeshObject::Create(ENV_SPHERE_MODEL));
		newObj->trl = glm::vec3(-50, -50, -50);
		newObj->scl = glm::vec3(30, 30, 30);

		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(0, -30, 50);
		newObj->scl = glm::vec3(30, 30, 10);
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(40, -45, 30);
		newObj->scl = glm::vec3(40, 10, 40);
		{
			worldRoot->NewChild(MeshObject::Create(ENV_STRING_MODEL)); // Needs collisions
			newObj->name = "Environment Curve Wall";
			newObj->trl = glm::vec3(45, 0, 45);
			newObj->offsetScl = glm::vec3(20, 50, 20);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(10, 25, 10), vec3(0, 0, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->SetPosition(newObj->trl);
		}
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(45, 35, 0);
		newObj->scl = glm::vec3(20, 30, 10);
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(0, 45, 25);
		newObj->scl = glm::vec3(30, 5, 50);
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(-20, -50, 10);
		newObj->scl = glm::vec3(50, 10, 30);
		{
			worldRoot->NewChild(MeshObject::Create(ENV_STRING_MODEL)); // Needs collisions
			newObj->name = "Environment Curve Wall 2";
			newObj->trl = glm::vec3(-50, 0, 10);
			newObj->offsetScl = glm::vec3(20, 50, 20);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(10, 25, 10), vec3(0, 0, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->SetPosition(newObj->trl);
		}
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(-50, 40, -40);
		newObj->scl = glm::vec3(30, 30, 40);
		//{
		//	worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Probably needs collision
		//	newObj->name = "Environment Wall";
		//	newObj->trl = glm::vec3(0, 0, -50);
		//	newObj->offsetScl = glm::vec3(30, 100, 10);

		//	newObj->AddPhysics(PhysicsObject::STATIC);
		//	auto physics = newObj->GetPhysics();

		//	physics->AddCollider(PhysicsObject::BOX, vec3(15, 50, 5), vec3(0, 0, 0));
		//	physics->SetBounciness(0.f);
		//	physics->SetFrictionCoefficient(0.5f);
		//	physics->SetPosition(newObj->trl);
		//}
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_MODEL));
		newObj->trl = glm::vec3(10, 0, -5);
		newObj->scl = glm::vec3(5, 50, 5);
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_MODEL));
		newObj->trl = glm::vec3(-30, 0, -20);
		newObj->scl = glm::vec3(5, 50, 5);
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_FLAT_MODEL));
		newObj->trl = glm::vec3(0, -49, 0);
		newObj->rot = glm::vec3(-90, 0, 0);
		newObj->scl = glm::vec3(100, 100, 1);
		
		float envStringRandTrlX[40] = { 0 };
		float envStringRandTrlZ[40] = { 0 };
		float envStringRandRotX[40] = { 0 };
		float envStringRandRotZ[40] = { 0 };

		for (int i = 0; i < 40; i++)
		{
			envStringRandTrlX[i] = rand() % 101 - 50;
			envStringRandTrlZ[i] = rand() % 101 - 50;
			
			envStringRandRotX[i] = rand() % 11 - 5;
			envStringRandRotZ[i] = rand() % 11 - 5;

			worldRoot->NewChild(MeshObject::Create(ENV_STRING_MODEL));
			newObj->trl = glm::vec3(envStringRandTrlX[i], 0, envStringRandTrlZ[i]);
			newObj->rot = glm::vec3(envStringRandRotX[i], 0, envStringRandRotZ[i]);
			newObj->scl = glm::vec3(0.8f, 0.8f, 0.8f);
		}
		
		worldRoot->NewChild(MeshObject::Create(NANOBOT_MODEL));
		newObj->trl = glm::vec3(-20, 5, 0);
		newObj->scl = glm::vec3(1, 1, 1);
	}

	// view space init
	{
		viewRoot->NewChild(MeshObject::Create(NANOBOT_MODEL));
		newObj->trl = glm::vec3(0.35f, -0.2f, -0.5f);
		newObj->scl = glm::vec3(0.1f, 0.1f, 0.1f);
	}

	// screen space init
	{
		screenRoot->NewChild(MeshObject::Create(UI_TEST, 1));  // create with 1 as UILayer, default 0
		newObj->trl = vec3(-0.8f, -0.8f, 0); // give any number for z, itll be force set to 0 in the loop
		newObj->scl = vec3(80, 80, 1); // give any number for z, itll be force set to 1 in the loop
		screenRoot->NewChild(MeshObject::Create(UI_TEST_2));
		newObj->trl = vec3(-0.85f, -0.85f, 0);
		newObj->scl = vec3(80, 80, 1);
		screenRoot->NewChild(MeshObject::Create(PNG_TEST));
		newObj->trl = vec3(-0.75f, -0.75f, 0);
		newObj->scl = vec3(80, 80, 1);

		// debug text
		InitDebugText(FONT_CASCADIA_MONO); // if you want another font for debug text, just change it to another font, tho dont call this in Update(), itll break
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

void SceneMedical::Update(double dt) {
	BaseScene::Update(dt);
	ClearDebugText();

	// fps calculation
	const float fpsUpdateTime = 0.5f;
	static float avgFps = 0;
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

	{ // Placeholder for Timer Display
		waveTimeAccumulator += dt;

		if (waveTimeAccumulator >= 1.0f)
		{
			waveTimeAccumulator -= 1.0f;
			waveTimeLeft--;

			if (waveTimeLeft <= 0.0f)
			{
				waveTimeLeft = 180;
			}
		}
		AddDebugText("Remaining Time For Wave: " + std::to_string(waveTimeLeft));
	}
	AddDebugText("Wave: " + std::to_string(waveNumber) + "/3");
	if (waveNumber == 1)
	{
		maxEntitiesP = 10;
		maxEntitiesAI = 5;
	}
	else if (waveNumber == 2)
	{
		maxEntitiesP = 15;
		maxEntitiesAI = 10;
	}
	else if (waveNumber == 3)
	{
		maxEntitiesP = 30;
		maxEntitiesAI = 20;
	}
	else
	{
		waveNumber = 1; // Set to default if somehow fails
		std::cout << "Wave may be incorrect" << std::endl;
	}
	AddDebugText("Bacteria Left: 10/" + std::to_string(maxEntitiesP));
	AddDebugText("Viruses Left: 5/" + std::to_string(maxEntitiesAI));

	// Temporary for now
	// When the Game State handler, the code snippet below will be stored properly
	DialogueManager::GetInstance().UpdateDialogue(dt);

	if (dt > 0.1f) {
		dt = 0.1f;
	}
	debugPhysicsTimer += dt;

	// simulation fps calculation
	static float simAvgFps = 0;
	{
		static float timer = 0;
		static int frameCount = 0;
		timer += dt;
		frameCount++;
		if (timer >= fpsUpdateTime) {
			simAvgFps = frameCount / timer;
			timer = 0;
			frameCount = 0;
		}
	}
	AddDebugText("average fps: " + std::to_string(avgFps) + ", simulation average fps: " + std::to_string(simAvgFps));

	auto& lightList = LightObject::lightList;
	auto& worldList = RObj::worldList;
	auto& viewList = RObj::viewList;
	auto& screenList = RObj::screenList;
	auto& physicsList = RObj::physicsList;

	// if you ever felt that you need dt inside HandleKeyPress(), that means you are doing smt wro- i mean, you need to use a variable to pass the info and commit changes in Update instead, HandleKeyPress() should not have those kinda logic inside it
	HandleKeyPress();

	// player updates
	{
		// update position and camera bobbing
		if (camera.GetCurrentMode() != Cam::MODE::FREE)
			player.UpdatePhysicsWithCamera(dt, camera);
		else
			player.UpdatePhysics(dt);
	}

	// camera
	/*camera.Update(dt);*/ // this must be right after player's block of code to make sure it is sync

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

		// btw. this code here visual does nothing, if you turn un debug and get close to to spot light and see it, youll realise its rotating perpendicularly to the light direction
		if (obj->name == "demo light spot") {
			obj->offsetRot.y += 45 * dt;
			obj->isDirty = true; // UpdateModel() cannot detect changes in offsets, so you need to manually set isDirty to true
		} // tho normally you wont need to touch offsets in Update() at all since you normally will have a group obj that is parented to this

		if (debug) {

		}

		if (!obj->GetPhysics())
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

		if (obj->geometryType == GROUP) {
			obj->allowRender = debug;
		}



		if (debug) {

		}

		if (!obj->GetPhysics())
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






		if (auto textObj = std::dynamic_pointer_cast<TextObject>(obj)) {
			if (textObj->name.find("dial_s") != std::string::npos) {
				if (DialogueManager::GetInstance().CheckActivePack()) {
					textObj->text = DialogueManager::GetInstance().GetCurrentSpeaker();
				}
				else
					textObj->text = "";
			}
			if (textObj->name.find("dial_t") != std::string::npos) {
				if (DialogueManager::GetInstance().CheckActivePack()) {
					textObj->text = DialogueManager::GetInstance().GetVisibleLine();
				}
				else
					textObj->text = "";
			}
			if (textObj->name.find("_debugtxt_") != std::string::npos) {
				textObj->allowRender = debug;
			}
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

	PhysicsEventListener& eventListener = PhysicsManager::GetInstance().GetEventListener();
	eventListener.UpdateEventValidity(PhysicsManager::GetInstance().GetWorld());
	PhysicsManager::GetInstance().UpdatePhysics(dt);

	const auto& debugRenderer = PhysicsManager::GetInstance().GetDebugRenderer();
	if (ALLOW_PHYSICS_DEBUG && renderDebugPhysics && debugRenderer && debugPhysicsTimer >= fpsUpdateTime) {
		debugPhysicsTimer -= fpsUpdateTime;
		debugPhysicsWorld = MeshBuilder::GenratePhysicsWorld(debugRenderer);
	}
	if (debugPhysicsTimer >= fpsUpdateTime * 2) {
		debugPhysicsTimer -= fpsUpdateTime;
	}

	{
		using CONTACT_EVENT = rp3d::CollisionCallback::ContactPair::EventType;
		using OVERLAP_EVENT = rp3d::OverlapCallback::OverlapPair::EventType; // for trigger events

		// physics objects
		for (unsigned i = 0; i < physicsList.size(); ) {
			if (physicsList[i].expired()) {
				physicsList.erase(physicsList.begin() + i);
				continue;
			}
			auto obj = physicsList[i].lock();
			auto physics = obj->GetPhysics();
			physics->InterpolateTransform();
			obj->UsePhysicsModel(); // physics objects' trl, rot and scl are disabled as they use the physics world's object's model, however the offset version still works (model only affect visual appearance)
			if (obj->name == "Environment Wall")
			{
				physics->contactEvent.Subscribe(
					[&](const rp3d::Body* other)
					{
						std::cout << "Wall contacted something!\n";
					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->contactEvent, rp3d::CollisionCallback::ContactPair::EventType::ContactStart));
			}
			i++;
		}
	}

	// player sync
	{
		player.SyncPhysics();
	}

	camera.Update(dt);
}


void SceneMedical::Render() {
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

void SceneMedical::Exit() {
	BaseScene::Exit();


}

void SceneMedical::HandleKeyPress() {

	if (debug) {
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_8)) {
			cullFaceActive = !cullFaceActive;
			if (cullFaceActive)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);
		}
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_9)) {
			wireFrameActive = !wireFrameActive;
			if (wireFrameActive)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

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

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_Q))
		{ // Test
			waveNumber++;
		}
	}
}


/*********************************************************************************************************************************************************************************/
/************************************************************************************ helpers ************************************************************************************/
/*********************************************************************************************************************************************************************************/


void SceneMedical::RenderObj(const std::shared_ptr<RObj> obj) {

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



void SceneMedical::RenderMesh(GEOMETRY_TYPE type, bool enableLight) {

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

void SceneMedical::InitDebugText(GEOMETRY_TYPE font) {
	auto& newObj = RObj::newObject;
	for (int i = 0; i < 10; i++) {
		screenRoot->NewChild(TextObject::Create("_debugtxt_" + std::to_string(i), "", vec3(0, 1, 0), font, false, 99));
		newObj->relativeTrl = true;
		newObj->trl = vec3(-0.98f, 0.95f - i * 0.05f, 0);
		newObj->scl = vec3(30, 30, 1);
		debugTextList.push_back(newObj);
	}
}

bool SceneMedical::AddDebugText(const std::string& text, int index) {

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

void SceneMedical::ClearDebugText() {
	for (auto& obj_weak : debugTextList)
		std::dynamic_pointer_cast<TextObject>(obj_weak.lock())->text = "";
}
