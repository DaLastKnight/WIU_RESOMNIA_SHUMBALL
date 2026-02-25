#define _USE_MATH_DEFINES
#include <cmath>

#include "SceneWhack.h"

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

#include <iostream>
#include <random>

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

SceneWhack::SceneWhack() {
}

SceneWhack::~SceneWhack() {
}

void SceneWhack::Init() {
	
	// set physics world settings
	auto& worldSettings = PhysicsManager::GetInstance().GetWorldSettingsObject();
	//worldSettings.gravity = rp3d::Vector3(0, -9.81f, 0); //this is the default gravity

	BaseScene::Init();

	// physics debug init
	{
		if (ALLOW_PHYSICS_DEBUG) {
			PhysicsManager::GetInstance().SetUpLogger("SceneDemo");
			PhysicsManager::GetInstance().SeteDebugRendering(true);
			PhysicsManager::GetInstance().SetDebugRenderItems(true, false, true, false, false);
		}
	}

	// directory init
	{
		AudioManager::GetInstance().SetDirectoryMUS("SceneWhack/Music");
		AudioManager::GetInstance().SetDirectorySFX("SceneWhack/SFX");
		TextureLoader::SetDirectory("SceneWhack/Image");
		ModelLoader::SetDirectory("SceneWhack/Model");
		DialogueManager::GetInstance().SetDirectory("SceneWhack/Dialogue");
	}

	// audio init
	{
		// music init
		AudioManager::GetInstance().LoadMUS("Wheel_Chill.ogg", 57.7555); // you need to input the total duration of the music in seconds as a double, sdl mixer cannot get the duration itself

		// sfx init
		AudioManager::GetInstance().LoadSFX(GOOFY_AHH_ASRIEL_STAR_SOUND, "sfx_asriel_star_drop.wav");

	}

	DialogueManager::GetInstance().LoadDialoguePack("Tutorial1.json");

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
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));
		meshList[DEBUG_LINE] = MeshBuilder::GenerateLine("debug line", 1);
		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTexture("skybox.tga"));
		
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTexture("circuit.tga"));
		meshList[PLATFORM] = MeshBuilder::GenerateGround("platform", 20, 5, TextureLoader::LoadTexture("circuit.tga"));
		meshList[TERMINAL] = MeshBuilder::GenerateOBJMTL("terminal", "terminal.obj", "terminal.mtl", TextureLoader::LoadTexture("terminal.png"));

		meshList[ANTIVIRUS_BALL] = MeshBuilder::GenerateSphere("antivirus ball", vec3(1.f), 0.5f, 16, 8, TextureLoader::LoadTexture("antivirus.tga"));
		meshList[TRIGGER_BOX] = MeshBuilder::GenerateCube("trigger box", vec3(1.f), 2);

		meshList[VIRUS_1] = MeshBuilder::GenerateOBJ("virus1", "virus_one.obj");
		meshList[VIRUS_2] = MeshBuilder::GenerateOBJ("virus2", "virus_two.obj");
		meshList[VIRUS_3] = MeshBuilder::GenerateOBJ("virus3", "virus_three.obj");

		meshList[PORTAL_EVIL] = MeshBuilder::GenerateQuad("portal evil", vec3(1), 1, 1, TextureLoader::LoadTexture("portal.tga"));
		meshList[PORTAL_GOOD] = MeshBuilder::GenerateQuad("portal good", vec3(1), 1, 1, TextureLoader::LoadTexture("portal.tga"));

		meshList[CROSSHAIR] = MeshBuilder::GenerateQuad("crosshair", vec3(1), 1, 1, TextureLoader::LoadTexture("crosshair.tga"));

		meshList[WHITE] = MeshBuilder::GenerateQuad("white", vec3(1), 1, 1, TextureLoader::LoadTexture("white.png"));
		meshList[GAUGE] = MeshBuilder::GenerateQuad("gauge", vec3(1), 1, 1, TextureLoader::LoadTexture("gauge.png"));
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

	// Collision categories:
	// 1: Player
	// 2: Ground
	// 3: TriggerBox
	// 4: Ball
	// 5: Virus
	// 6: Portal

	// init default stats
	{
		RObj::setDefaultStat.Subscribe(AXES, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT); // does not get affected by light, always bright (fog does not work on objects not affected by light)
			});
		RObj::setDefaultStat.Subscribe(LIGHT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON); // bright when shinned with light directly and and still be rather bright when not shinned
			obj->offsetScl = vec3(0.05f);
			});
		RObj::setDefaultStat.Subscribe(GROUP, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);
			});
		RObj::setDefaultStat.Subscribe(DEBUG_LINE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT);
			});
		RObj::setDefaultStat.Subscribe(FONT_CASCADIA_MONO, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(SKYBOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT); // affected by light, tho the material is set in a way so that it is always bright, just like NO_LIGHT (this makes sure fog can still be casted on it while be bright at times without fog)
			});


		RObj::setDefaultStat.Subscribe(GROUND, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);

			obj->AddPhysics(PhysicsObject::STATIC); // takes in PhysicsObject::BODY_TYPE
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(500, 0.5f, 500), vec3(0, -0.5f, 0));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_3 | CATEGORY_4);
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->UpdateMassProperties();

			});
		RObj::setDefaultStat.Subscribe(PLATFORM, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);
			obj->colorFilter = vec3(1.f, 0.f, 1.f);
			obj->alpha = 0.7f;
			obj->hasTransparency = true;

			obj->AddPhysics(PhysicsObject::STATIC); // takes in PhysicsObject::BODY_TYPE
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(10, 0.5f, 10), vec3(0, -0.5f, 0));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_3 | CATEGORY_4);
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->UpdateMassProperties();

			});
		RObj::setDefaultStat.Subscribe(TERMINAL, [](const std::shared_ptr<RObj>& obj) {
			obj->colorFilter = vec3(0.4f, 0.4f, 0.6f);
			obj->AddPhysics(PhysicsObject::STATIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(0.5f, 0.8f, 0.5f), vec3(0.f, 0.8f, 0.f));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 | CATEGORY_3);
			physics->UpdateMassProperties();
			});
		

		RObj::setDefaultStat.Subscribe(ANTIVIRUS_BALL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);

			obj->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::SPHERE, vec3(0.5f, 0, 0));
			physics->SetBounciness(0.1f);
			physics->SetFrictionCoefficient(0.2f);
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_4);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_2 | CATEGORY_3 | CATEGORY_4 | CATEGORY_5);
			physics->UpdateMassProperties(); // must call this after getting all colliders set, if you set another collider after this, you have to call this again
			physics->SetPosition(vec3(0, 5, 0));

			});
		RObj::setDestroyedEvent.Subscribe(ANTIVIRUS_BALL, [&](const std::shared_ptr<RObj>& obj) { // these specific lines is required for all physics object that can be deleted in the programe, its safe to add it for all physics objects to be safe in case if you missed any
			dirtyWorldList = true;
			});
		RObj::setDefaultStat.Subscribe(TRIGGER_BOX, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);

			obj->AddPhysics(PhysicsObject::STATIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(1.f, 1.f, 1.f));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_3);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1);
			physics->SetTrigger(true);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(TRIGGER_BOX, [&](const std::shared_ptr<RObj>& obj) {
			dirtyWorldList = true;
			});
		

		RObj::setDefaultStat.Subscribe(VIRUS_1, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::MATT);
			obj->colorFilter = vec3(1, 0, 0);
			obj->offsetScl = vec3(0.02f, 0.02f, 0.02f);
			obj->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = obj->GetPhysics();
			physics->Getbody()->enableGravity(false);
			physics->AddCollider(PhysicsObject::BOX, vec3(1.f, 0.7f, 0.4f), vec3(0.f, 0.7f, 0.f));
			physics->SetBounciness(1.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_5);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_4 | CATEGORY_6);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(VIRUS_1, [&](const std::shared_ptr<RObj>& obj) {
			gameScore++;
			dirtyWorldList = true;
			});


		RObj::setDefaultStat.Subscribe(PORTAL_GOOD, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT);
			obj->offsetScl = vec3(5.f);
			obj->offsetRot.y = -90.f;
			obj->hasTransparency = true;
			obj->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = obj->GetPhysics();
			physics->Getbody()->enableGravity(false);
			physics->AddCollider(PhysicsObject::BOX, vec3(0.5f, 2.5f, 2.5f));
			physics->SetBounciness(1.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_6);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_5);
			physics->SetTrigger(true);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(PORTAL_GOOD, [&](const std::shared_ptr<RObj>& obj) {
			dirtyWorldList = true;
			});


		RObj::setDefaultStat.Subscribe(PORTAL_EVIL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT);
			obj->offsetScl = vec3(5.f);
			obj->offsetRot.y = -90.f;
			obj->hasTransparency = true;
			obj->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = obj->GetPhysics();
			physics->Getbody()->enableGravity(false);
			physics->SetCollisionActive(false);
			physics->SetBounciness(1.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(PORTAL_EVIL, [&](const std::shared_ptr<RObj>& obj) {
			dirtyWorldList = true;
			});
		RObj::setDefaultStat.Subscribe(CROSSHAIR, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NO_LIGHT);
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});


		RObj::setDefaultStat.Subscribe(WHITE, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAUGE, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		
	}

	auto& newObj = RObj::newObject;
	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(AXES));

		worldRoot->NewChild(MeshObject::Create(GROUND));
		worldRoot->NewChild(MeshObject::Create(PLATFORM));
		newObj->GetPhysics()->SetPosition(vec3(0.f, 6.f, 0.f));

		worldRoot->NewChild(MeshObject::Create(SKYBOX));

		worldRoot->NewChild(MeshObject::Create(TERMINAL));
		newObj->name = "terminal";
		newObj->GetPhysics()->SetPosition(vec3(8.f, 6.f, 0.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, -90.f, 0.f));

		InitPortalMaps();
		ChangePortalMap();

		worldRoot->NewChild(MeshObject::Create(CROSSHAIR));
		newObj->name = "crosshair";
		newObj->scl = vec3(3);
		newObj->trl = vec3(0, 1.f, 0);
		newObj->rot.y = -90.f;

		worldRoot->NewChild(MeshObject::Create(PORTAL_GOOD));
		newObj->name = "portal_ground";
		newObj->offsetScl = vec3(40.f);
		newObj->GetPhysics()->SetPosition(vec3(50.f, 0.5f, 0.f));
		newObj->GetPhysics()->SetAllowSleep(false);
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 0.f, -90.f));
		newObj->GetPhysics()->Getbody()->removeCollider(newObj->GetPhysics()->Getbody()->getCollider(0));
		newObj->GetPhysics()->AddCollider(PhysicsObject::BOX, vec3(0.5f, 20.f, 20.f));
		newObj->GetPhysics()->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_6);
		newObj->GetPhysics()->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_5);
		newObj->GetPhysics()->SetTrigger(true);
		newObj->GetPhysics()->UpdateMassProperties();

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "gamestart_box";
		newObj->alpha = 0.f;
		newObj->hasTransparency = true;
		newObj->GetPhysics()->SetPosition(vec3(8.f, 6.5f, 0.f));

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
	}

	// view space init
	{
		
	}

	// screen space init
	{
		screenRoot->NewChild(MeshObject::Create(GAUGE));
		newObj->name = "gauge";
		newObj->trl = vec3(0.f, -0.7f, 0); // give any number for z, itll be force set to 0 in the loop
		newObj->scl = vec3(400, 100, 1); // give any number for z, itll be force set to 1 in the loop

		screenRoot->NewChild(MeshObject::Create(WHITE, 1)); // create with 1 as UILayer, default 0
		newObj->name = "marker";
		newObj->trl = vec3(0.f, -0.7f, 0);
		newObj->scl = vec3(20, 120, 1);

		screenRoot->NewChild(TextObject::Create("dial_speaker", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0, -0.5f, 0);
		newObj->scl = vec3(30, 30, 1);
		screenRoot->NewChild(TextObject::Create("dial_text", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0, -0.575f, 0);
		newObj->scl = vec3(30, 30, 1);

		screenRoot->NewChild(TextObject::Create("score", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0.7f, 0.8f, 0);
		newObj->scl = vec3(80, 80, 1);

		screenRoot->NewChild(TextObject::Create("gameTimer", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0.7f, 0.6f, 0);
		newObj->scl = vec3(80, 80, 1);

		// debug text
		InitDebugText(FONT_CASCADIA_MONO); // if you want another font for debug text, just change it to another font, tho dont call this in Update(), itll break
	}

	/************************ bellow for external class inits ************************/
	{
		// camera init
		camera.Init(glm::vec3(1, 1.5f, -1));
		camera.Set(FPCamera::MODE::FIRST_PERSON);

		// player init
		player.Init(worldRoot, GROUP, vec3(0, 0.5f, 0));
		player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0.f, 8.f, 0.f));
		//player.renderGroup.lock()->GetPhysics()->Getbody()->enableGravity(false);
		player.renderGroup.lock()->GetPhysics()->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_1);
		player.renderGroup.lock()->GetPhysics()->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_2 | CATEGORY_3);

		/*player.renderGroup.lock()->NewChild(MeshObject::Create(VIRUS_2));
		vec3 virusToTarget = newObj->trl - player.position;
		virusToTarget = glm::normalize(virusToTarget);
		float desiredYawDeg = glm::degrees(atan2f(virusToTarget.x, virusToTarget.z));
		newObj->offsetRot = vec3(0.f, desiredYawDeg, 0.f);*/
	}

	RObj::newObject.reset();
}

void SceneWhack::Update(double dt) {
	BaseScene::Update(dt);
	ClearDebugText();

	if (gamePaused)
	{
		dt = 0;
	}

	// fps calculation
	const float fpsUpdateTime = 0.5f;
	static float avgFps = 0;
	{
		static float timer = 0;
		static int frameCount = 0;
		timer += dt;
		frameCount++;
		if (timer >= fpsUpdateTime) {
			avgFps = frameCount / timer;
			timer = 0;
			frameCount = 0;
		}
	}

	DialogueManager::GetInstance().UpdateDialogue(dt);

	if (dt > 0.1f) {
		dt = 0.1f;
	}
	debugPhysicsTimer += dt;
	elapsed += dt;

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

	auto& newObj = RObj::newObject;
	elapsedVirusInterval -= dt;
	if (elapsedVirusInterval <= 0)
	{
		worldRoot->NewChild(MeshObject::Create(VIRUS_1));
		newObj->name = "virus";
		newObj->GetPhysics()->Getbody()->enableGravity(false);
		
		static std::mt19937 solsrng(std::random_device{}());
		std::uniform_int_distribution<int> portalDistribution(0, portalEvilPositionsList.size() - 1);
		int spawnIndex = portalDistribution(solsrng);
		newObj->GetPhysics()->SetPosition(portalEvilPositionsList[spawnIndex]);
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 90.f, 0.f));

		Virus newVirus(newObj);
		newVirus.waypoints = newVirus.BuildRandomPath(portalEvilPositionsList, solsrng, portalDistribution, spawnIndex, true);
		newVirus.waypointIndex = 0;
		virusList.push_back(newVirus);
		elapsedVirusInterval = 3;
	}

	gameTimer -= dt;

	// player updates
	{
		//player.renderGroup.lock()->offsetRot.y += 100.f * dt;
		
		// update position and camera bobbing
		if (camera.GetCurrentMode() != Cam::MODE::FREE)
			player.UpdatePhysicsWithCamera(dt, camera);
		else
			player.UpdatePhysics(dt);
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

		if (obj->name == "crosshair")
		{
			obj->trl = selectedPortalPosition - vec3(5.f, 0.f, 0.f);
			obj->rot.z += 45.f * dt;
			/*vec3 toTarget = obj->trl - player.position;
			toTarget.y = 0.f;
			toTarget = glm::normalize(toTarget);
			float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
			obj->rot.y = desiredYawDeg + 90.f;*/
		}

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

		if (obj->name == "gauge")
		{
			obj->allowRender = false;
			if (isGaugeActive) obj->allowRender = true;
		}

		if (obj->name == "marker")
		{
			if (isGaugeActive)
			{
				obj->allowRender = true;
				gaugeMarkerPosition.x = obj->trl.x = -0.225f * cosf(4.f * (float)elapsed);
			}
			else
			{
				gaugeMarkerPosition.x  = obj->trl.x = -0.225f;
				obj->allowRender = false;
			}
		}

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
			if (textObj->name.find("score") != std::string::npos)
			{
				textObj->text = std::to_string(gameScore);
			}
			if (textObj->name.find("gameTimer") != std::string::npos)
			{
				textObj->text = std::to_string(static_cast<int>(gameTimer));
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

	// update physics
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
		PhysicsEventListener& eventListener = PhysicsManager::GetInstance().GetEventListener();
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

			if (obj->name == "gamestart_box") {
				physics->triggerEvent.Subscribe([&](const rp3d::Body* overlapped) {
					
					if (player.renderGroup.lock()->GetPhysics()->Getbody() == overlapped)
					{
						camera.Set(FPCamera::MODE::LOCK_ON);
						camera.SetDirection(vec3(20.f, 4.f, 0.f) - camera.GetPlainPosition());
					}
					
					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->triggerEvent, OVERLAP_EVENT::OverlapStay)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
				physics->triggerEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
			}

			if (obj->name == "portal_ground")
			{
				obj->offsetRot.x += 45.f * dt;
				
				physics->triggerEvent.Subscribe([&](const rp3d::Body* overlapped) {

					for (unsigned j = 0; j < virusList.size();)
					{
						if (virusList[j].virus.expired()) {
							virusList.erase(virusList.begin() + j);
							continue;
						}

						auto virusObject = virusList[j].virus.lock();
						auto virusPhysics = virusObject->GetPhysics();

						if (virusPhysics->Getbody() == overlapped)
						{
							virusList[j].hasBeenDestroyed = true;
						}

						j++;
					}

					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->triggerEvent, OVERLAP_EVENT::OverlapStart)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
				physics->triggerEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
			}

			if (obj->name == "portal_evil")
			{
 				int portalIndex = FindEvilPortalIndex(obj);
				if (portalIndex != -1)
				{
					physics->SetPosition(vec3(portalEvilList[portalIndex].portalBasePosition.x + 1.f * sinf((float)elapsed * 2.f), physics->GetPosition().y, physics->GetPosition().z));
					obj->offsetRot.z += 45.f * dt;
					vec3 toTarget = physics->GetPosition() - player.position;
					toTarget.y = 0.f;
					toTarget = glm::normalize(toTarget);
					float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
					obj->offsetRot.y = desiredYawDeg + 180.f;
				}
			}

			if (obj->name == "portal_good")
			{
				physics->triggerEvent.Subscribe([&](const rp3d::Body* overlapped) {

					for (unsigned j = 0; j < virusList.size();)
					{
						if (virusList[j].virus.expired()) {
							virusList.erase(virusList.begin() + j);
							continue;
						}

						auto virusObject = virusList[j].virus.lock();
						auto virusPhysics = virusObject->GetPhysics();

						if (virusPhysics->Getbody() == overlapped)
						{
							virusList[j].hasBeenDestroyed = true;
						}

						j++;
					}

					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->triggerEvent, OVERLAP_EVENT::OverlapStart)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
				physics->triggerEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
				
				int portalIndex = FindGoodPortalIndex(obj);
				if (portalIndex != -1)
				{
					physics->SetPosition(vec3(portalGoodList[portalIndex].portalBasePosition.x + 1.f * sinf((float)elapsed * 2.f), physics->GetPosition().y, physics->GetPosition().z));
					obj->offsetRot.z += 45.f * dt;
					vec3 toTarget = physics->GetPosition() - player.position;
					toTarget.y = 0.f;
					toTarget = glm::normalize(toTarget);
					float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
					obj->offsetRot.y = desiredYawDeg + 180.f;
				}
			}

			if (obj->name == "antivirus ball")
			{
				int timerIndex = FindTimerIndex(obj);
				if (timerIndex != -1)
				{
					ballTimeList[timerIndex].timer -= static_cast<float>(dt);

					if (ballTimeList[timerIndex].timer <= 0.f)
					{
						ballTimeList.erase(ballTimeList.begin() + timerIndex);
						physicsList.erase(physicsList.begin() + i);
						obj->Destroy();
						obj.reset();
						continue;
					}

					physics->contactEvent.Subscribe([&](const rp3d::Body* contacted) {

						for (unsigned j = 0; j < virusList.size();)
						{
							if (virusList[j].virus.expired()) {
								virusList.erase(virusList.begin() + j);
								continue;
							}

							auto virusObject = virusList[j].virus.lock();
							auto virusPhysics = virusObject->GetPhysics();
							
							if (virusPhysics->Getbody() == contacted)
							{
								virusList[j].hasBeenHit = true;
							}

							j++;
						}
						
						});
					eventListener.AddToContactEvents(PEvent(physics, physics->contactEvent, CONTACT_EVENT::ContactStart)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
					physics->contactEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
				}
			}

			if (obj->name == "virus")
			{
				int virusIndex = FindVirusIndex(obj);
				if (virusIndex != -1)
				{
					if (virusList[virusIndex].hasBeenDestroyed)
					{
						virusList.erase(virusList.begin() + virusIndex);
						physicsList.erase(physicsList.begin() + i);
						obj->Destroy();
						obj.reset();
						continue;
					}

					auto& currentVirus = virusList[virusIndex];
					if (currentVirus.waypoints.empty())
					{
					
					}
					else
					{
						if (!currentVirus.hasBeenHit)
						{
							currentVirus.virus.lock()->GetPhysics()->SetOrientation(vec3(0.f, 90.f, 0.f));
							vec3 virusPosition = physics->GetPosition();
							vec3 targetPosition = currentVirus.waypoints[currentVirus.waypointIndex];

							vec3 virusToTarget = targetPosition - virusPosition;

							float distanceSquared = glm::dot(virusToTarget, virusToTarget);
							float arriveRadius = currentVirus.arriveRadius;

							if (distanceSquared <= arriveRadius * arriveRadius)
							{
								currentVirus.waypointIndex++;
								if (currentVirus.waypointIndex >= currentVirus.waypoints.size())
								{
									currentVirus.waypointIndex = 0;
								}

								targetPosition = currentVirus.waypoints[currentVirus.waypointIndex];
								virusToTarget = targetPosition - virusPosition;
								distanceSquared = glm::dot(virusToTarget, virusToTarget);
							}

							if (distanceSquared > 0.000001f)
							{
								vec3 direction = virusToTarget / sqrtf(distanceSquared);
								vec3 desiredVelocity = direction * currentVirus.moveSpeed;
								vec3 currentVelocity = physics->GetVelocity();
								vec3 newVelocity = Smooth(currentVelocity, desiredVelocity, currentVirus.smoothing, (float)dt);
								physics->SetVelocity(newVelocity);
							}
						}
						else
						{
							currentVirus.timerHit -= dt;
							if (currentVirus.timerHit <= 0.f)
							{
								currentVirus.timerHit = 5.f;
								currentVirus.hasBeenHit = false;
							}
						}
						
						
					}
				}
			}

			i++;
		}
	}

	// player sync
	{
		player.SyncPhysics();
	}

	// camera
	camera.Update(dt); // this must be right after player's block of code to make sure it is sync

	// yah you can do this to add text, but this must be called every frame since it gets refreshed every frame
	// you can call AddDebugText() at anywhere after calling BaseScene::Update(); and before calling renderObjectList(RObj::screenList, true); and itll work
	AddDebugText("camera.basePosition: " + VecToString(camera.basePosition)); // VecToString supports vec2, vec3 and vec4 (idfk why i didt that but why not ig)
	AddDebugText("camera.finalPosition: " + VecToString(camera.GetPlainPosition()));
	AddDebugText("player.physics.postion: " + VecToString(player.renderGroup.lock()->GetPhysics()->GetPosition()));
	AddDebugText("player.physics.velocity: " + VecToString(player.renderGroup.lock()->GetPhysics()->GetVelocity()));
	AddDebugText("gaugeMarker.position: " + VecToString(gaugeMarkerPosition));
	AddDebugText("mouseX: " + std::to_string(MouseController::GetInstance()->GetMousePositionX()));
	AddDebugText("mouseY: " + std::to_string(MouseController::GetInstance()->GetMousePositionY()));

	if (dirtyWorldList) {
		for (unsigned i = 0; i < worldList.size(); ) {
			if (worldList[i].expired()) {
				worldList.erase(worldList.begin() + i);
				continue;
			}
			i++;
		}
	}

	RObj::newObject.reset();
}


void SceneWhack::Render() {
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

	// render debug physics
	if (ALLOW_PHYSICS_DEBUG && renderDebugPhysics && debugPhysicsWorld) {
		modelStack.Clear();
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

		debugPhysicsWorld->RenderPhysicsWorld();
	}


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

void SceneWhack::Exit() {
	BaseScene::Exit();
	delete portalGround;

}

void SceneWhack::HandleKeyPress() {

	static GAMESTATE currentGameState = FREEROAM;
	
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

	// debug keys
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_0))
	{
		if (gamePaused)
		{
			gamePaused = false;
		}
		else
		{
			gamePaused = true;
		}
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
		debug = !debug;
		renderDebugPhysics = false;
		camera.Set(Cam::MODE::FIRST_PERSON);
		player.allowControl = true;
	}
	if (debug) {
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_C)) {
			if (camera.GetCurrentMode() != Cam::MODE::FREE) {
				camera.Set(Cam::MODE::FREE);
				player.allowControl = false;
			}
			else {
				camera.Set(Cam::MODE::FIRST_PERSON);
				player.allowControl = true;
			}
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_P)) {
			renderDebugPhysics = !renderDebugPhysics;
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_LEFT_ALT)) {
		static auto prevMode = Cam::MODE::FREE;
		auto& cameraMode = camera.GetCurrentMode();

		if (cameraMode != Cam::MODE::PAUSE) {
			prevMode = cameraMode;
			camera.Set(Cam::MODE::PAUSE);
			player.allowControl = false;
		}
		else {
			camera.Set(prevMode);
			player.allowControl = true;
		}
	}

	static int portalIndex = 0;

	switch (currentGameState)
	{
	case GAMEPLAY:
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_5))
		{
			ChangePortalMap();
			portalIndex = 0;
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_A))
		{
			selectedPortalPosition = portalGoodList[portalIndex].portalBasePosition;
			//camera.SetDirection(selectedPortalPosition - camera.GetPlainPosition());
			portalIndex--;
			if (portalIndex < 0)
			{
				portalIndex = portalGoodPositionsList.size() - 1;
			}
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_D))
		{
			selectedPortalPosition = portalGoodList[portalIndex].portalBasePosition;
			//camera.SetDirection(selectedPortalPosition - camera.GetPlainPosition());
			portalIndex++;
			if (portalIndex % portalGoodPositionsList.size() == 0 && portalIndex != 0)
			{
				portalIndex = 0;
			}
		}
		
		if (MouseController::GetInstance()->IsButtonDown(MouseController::LMB)) {
			isGaugeActive = true;
		}
		else
		{
			isGaugeActive = false;
		}
		if (MouseController::GetInstance()->IsButtonReleased(MouseController::LMB))
		{
			isGaugeActive = false;

			worldRoot->NewChild(MeshObject::Create(ANTIVIRUS_BALL));
			vec3 ballSpawnPosition = selectedPortalPosition - vec3(15.f, 0.f, 0.f);
			vec3 direction = selectedPortalPosition - ballSpawnPosition;
			direction = glm::normalize(direction);
			auto& newObj = RenderObject::newObject;
			ballTimeList.push_back({ 2.f, std::weak_ptr<RenderObject>(newObj) });
			newObj->name = "antivirus ball";
			auto physics = newObj->GetPhysics();
			physics->SetPosition(ballSpawnPosition);
			physics->Getbody()->enableGravity(false);

			float impulseStrength = 0.f;
			if (gaugeMarkerPosition.x < 0.05f && gaugeMarkerPosition.x > -0.05f)
			{
				impulseStrength = 40000.f;
			}
			else if (gaugeMarkerPosition.x < 0.12f && gaugeMarkerPosition.x > -0.12f)
			{
				impulseStrength = 25000.f;
			}
			else if (gaugeMarkerPosition.x < 0.225f && gaugeMarkerPosition.x > -0.225f)
			{
				impulseStrength = 10000.f;
			}

			physics->AddImpulse(impulseStrength * direction);
			physics->AddTorque(vec3(5.f, 30.f, 0.f));
		}


		break;
	case FREEROAM:
		if (player.allowControl)
		{
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
		}
		break;
	case DIALOGUE:
		// dialogue controls
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE))
		{
			if (DialogueManager::GetInstance().CheckActivePack())
			{
				DialogueManager::GetInstance().ControlCurrentDialogue();
			}
		}
		break;
	default:
		break;
	}

	

	if (MouseController::GetInstance()->IsButtonPressed(MouseController::LMB)) {
		//AudioManager::GetInstance().PlayMUS(0, 1);
	}

	// player controls
	if (player.allowControl) {

		

		// action
		

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

		// fake jump lol
		/*if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE)) {
			player.renderGroup.lock()->GetPhysics()->AddSoftImpulse(vec3(0, 10000, 0));
		}*/

		

		
	}

	static bool inGame = false;
	static bool changed = false;

	if (changed)
	{
		if (inGame)
		{
			player.renderGroup.lock()->GetPhysics()->Getbody()->enableGravity(false);
			player.renderGroup.lock()->GetPhysics()->SetVelocity(vec3(player.renderGroup.lock()->GetPhysics()->GetVelocity().x, 0.f, player.renderGroup.lock()->GetPhysics()->GetVelocity().z));
			camera.Set(Cam::MODE::LOCK_ON);
			player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(15.f, 10.f, 0.f));
			camera.SetDirection(vec3(20.f, 6.5f, 0.f) - vec3(10.f, 6.5f, 0.f));
			
			player.allowControl = false;
		}
		else
		{
			camera.Set(Cam::MODE::FIRST_PERSON);
			player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0.f, 6.5f, 0.f));
			player.allowControl = true;
		}

		changed = false;
	}
}


/*********************************************************************************************************************************************************************************/
/************************************************************************************ helpers ************************************************************************************/
/*********************************************************************************************************************************************************************************/


void SceneWhack::RenderObj(const std::shared_ptr<RObj> obj) {

	if (!obj->allowRender)
		return;

	bool enableLight = true;
	if (obj->material.type == Material::NO_LIGHT || obj->renderType == RObj::SCREEN)
		enableLight = false;

	Material meshMaterial = meshList[obj->geometryType]->material;
	if (obj->material.type != Material::MESH_MATERIAL) {
		meshList[obj->geometryType]->material = obj->material;
	}

	glUniform3fv(m_parameters[U_COLOR_FILTER], 1, &obj->accumulatedColorFilter.r);
	glUniform1f(m_parameters[U_COLOR_ALPHA], obj->accumulatedAlpha);

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



void SceneWhack::RenderMesh(GEOMETRY_TYPE type, bool enableLight) {

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

void SceneWhack::InitDebugText(GEOMETRY_TYPE font) {
	auto& newObj = RObj::newObject;
	for (int i = 0; i < 10; i++) {
		screenRoot->NewChild(TextObject::Create("_debugtxt_" + std::to_string(i), "", vec3(0, 1, 0), font, false, 99));
		newObj->relativeTrl = true;
		newObj->trl = vec3(-0.98f, 0.95f - i * 0.05f, 0);
		newObj->scl = vec3(30, 30, 1);
		debugTextList.push_back(newObj);
	}
}

bool SceneWhack::AddDebugText(const std::string& text, int index) {

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

void SceneWhack::ClearDebugText() {
	for (auto& obj_weak : debugTextList)
		std::dynamic_pointer_cast<TextObject>(obj_weak.lock())->text = "";
}

void SceneWhack::InitPortalMaps()
{
	portalEvilMapList[0] = { vec3(40.f, 15.f, -5.f), vec3(40.f, 15.f, 5.f), vec3(40.f, 5.f, -5.f), vec3(40.f, 5.f, 5.f) };
	portalEvilMapList[1] = { vec3(40.f, 15.f, 0.f), vec3(40.f, 10.f, -10.f), vec3(40.f, 10.f, 10.f),  vec3(40.f, 5.f, 0.f) };

	portalGoodMapList[0] = { vec3(43.f, 15.f, 0.f), vec3(43.f, 10.f, -5.f), vec3(43.f, 10.f, 0.f), vec3(43.f, 10.f, 5.f),  vec3(43.f, 5.f, 0.f) };
	portalGoodMapList[1] = { vec3(43.f, 15.f, -5.f), vec3(43.f, 15.f, 5.f), vec3(43.f, 10.f, -5.f), vec3(43.f, 10.f, 0.f), vec3(43.f, 10.f, 5.f), vec3(43.f, 5.f, -5.f), vec3(43.f, 5.f, 5.f) };
}

void SceneWhack::ChangePortalMap()
{
	auto& physicsList = RObj::physicsList;
	for (unsigned i = 0; i < physicsList.size();)
	{
		if (physicsList[i].expired()) {
			physicsList.erase(physicsList.begin() + i);
			continue;
		}
		
		auto obj = physicsList[i].lock();
		auto physics = obj->GetPhysics();
		
		if (obj->name == "portal_good")
		{
			int portalIndex = FindGoodPortalIndex(obj);
 			if (portalIndex != -1)
			{
 				portalGoodList.erase(portalGoodList.begin() + portalIndex);
				physicsList.erase(physicsList.begin() + i);
				obj->Destroy();
				obj.reset();
				continue;
			}
		}

		if (obj->name == "portal_evil")
		{
			int portalIndex = FindEvilPortalIndex(obj);
			if (portalIndex != -1)
			{
				portalEvilList.erase(portalEvilList.begin() + portalIndex);
				physicsList.erase(physicsList.begin() + i);
				obj->Destroy();
				obj.reset();
				continue;
			}
		}

		if (obj->name == "virus")
		{
			int virusIndex = FindVirusIndex(obj);
			if (virusIndex != -1)
			{
				auto& currentVirus = virusList[virusIndex];
				currentVirus.waypoints.erase(currentVirus.waypoints.begin(), currentVirus.waypoints.end());
				currentVirus.waypointIndex = 0;
			}
		}
		i++;
	}

	for (unsigned i = 0; i < portalEvilList.size();)
	{
		if (portalEvilList[i].portal.expired()) {
			portalEvilList.erase(portalEvilList.begin() + i);
			continue;
		}

		i++;
	}

	for (unsigned i = 0; i < portalGoodList.size();)
	{
		if (portalGoodList[i].portal.expired()) {
			portalGoodList.erase(portalGoodList.begin() + i);
			continue;
		}

		i++;
	}

	if (!portalEvilPositionsList.empty()) portalEvilPositionsList.erase(portalEvilPositionsList.begin(), portalEvilPositionsList.end());
	if (!portalGoodPositionsList.empty()) portalGoodPositionsList.erase(portalGoodPositionsList.begin(), portalGoodPositionsList.end());
	
	static std::mt19937 portalrng(std::random_device{}());
	std::uniform_int_distribution<int> mapDistribution(0, 1);
	int randomMapIndex = mapDistribution(portalrng);

	portalEvilPositionsList = portalEvilMapList[randomMapIndex];
	portalGoodPositionsList = portalGoodMapList[randomMapIndex];

	auto& newObj = RObj::newObject;
	for (int i = 0; i < portalEvilPositionsList.size(); i++)
	{
		worldRoot->NewChild(MeshObject::Create(PORTAL_EVIL));
		newObj->name = "portal_evil";
		newObj->GetPhysics()->SetPosition(portalEvilPositionsList[i]);
		newObj->colorFilter = vec3(1.f, 0.1f, 0.1f);

		portalEvilList.push_back({ portalEvilPositionsList[i], newObj });
	}

	for (int i = 0; i < portalGoodPositionsList.size(); i++)
	{
		worldRoot->NewChild(MeshObject::Create(PORTAL_GOOD));
		newObj->name = "portal_good";
		newObj->GetPhysics()->SetPosition(portalGoodPositionsList[i]);

		portalGoodList.push_back({ portalGoodPositionsList[i], newObj });
	}

	std::uniform_int_distribution<int> portalDistribution(0, portalEvilPositionsList.size() - 1);

	for (unsigned i = 0; i < physicsList.size();)
	{
		auto obj = physicsList[i].lock();
		auto physics = obj->GetPhysics();
		
		if (obj->name == "virus")
		{
			int virusIndex = FindVirusIndex(obj);
			if (virusIndex != -1)
			{
				
				auto& currentVirus = virusList[virusIndex];
				currentVirus.waypoints = currentVirus.BuildRandomPath(portalEvilPositionsList, portalrng, portalDistribution, -1, false);
				currentVirus.waypointIndex = 0;
			}
		}
		
		i++;
	}

	selectedPortalPosition = portalGoodList[0].portalBasePosition;
}

void SceneWhack::StartCutscene()
{
	static int cutsceneIndex = 0;

	// maybe add enums for each cutscene later
	if (!hasFinishedTutorial)
	{
		switch (cutsceneIndex)
		{
		case 0:
			if (DialogueManager::GetInstance().CheckActivePack()) return;
			player.allowControl = false;
			camera.Set(FPCamera::MODE::LOCK_ON);
			camera.SetDirection(vec3(20.f, 4.f, 0.f) - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("Tutorial1");
			cutsceneIndex++;
			break;
		case 1:
			if (DialogueManager::GetInstance().CheckActivePack()) return;
			player.allowControl = false;
			camera.SetDirection(vec3(20.f, 4.f, 5.f) - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("Tutorial1");
			cutsceneIndex++;
			break;
		case 2:
			if (DialogueManager::GetInstance().CheckActivePack()) return;
			hasFinishedTutorial = true;
			camera.Set(Cam::MODE::FIRST_PERSON);
			player.allowControl = true;
			cutsceneIndex = 0;
			break;
		}
	}
}
