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


/*****************************************************************************************************************************************************************************************/
/************************************************************************************ scene functions ************************************************************************************/
/*****************************************************************************************************************************************************************************************/

SceneRhythm::SceneRhythm() 
	: currentState(LOAD_IN) {}

SceneRhythm::~SceneRhythm() {}

void SceneRhythm::Init() {
	// set physics world settings
	auto& worldSettings = PhysicsManager::GetInstance().GetWorldSettingsObject();
	//worldSettings.gravity = rp3d::Vector3(0, -9.81, 0); //this is the default gravity

	BaseScene::Init();

	// physics debug init
	{
		if (ALLOW_PHYSICS_DEBUG) {
			PhysicsManager::GetInstance().SetUpLogger("SceneRhythm");
			PhysicsManager::GetInstance().SeteDebugRendering(true);
			PhysicsManager::GetInstance().SetDebugRenderItems(true, false, true, false, false);
		}
	}

	// directory init
	{
		AudioManager::GetInstance().SetDirectoryMUS("SceneRhythm/Music");
		AudioManager::GetInstance().SetDirectorySFX("SceneRhythm/SFX");
		TextureLoader::SetDirectory("SceneRhythm/Image");
		ModelLoader::SetDirectory("SceneRhythm/Model");
		DialogueManager::GetInstance().SetDirectory("SceneRhythm/Dialogue");
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
		atmosphere.Set(vec3(0.05f, 0.05f, 0.05f), 1, 2, 1, 10);
		UpdateAtmosphereUniform();
	}	

	// Init VBO here
	{
		for (int i = 0; i < static_cast<int>(TOTAL); ++i)
		{
			meshList[i] = nullptr;
		}

		auto blackTexture = TextureLoader::LoadTexture("black.png");

		meshList[AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, blackTexture);
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox", blackTexture);
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));

		meshList[BLACK] = MeshBuilder::GenerateQuad("black", vec3(), 1, 1, blackTexture);

		meshList[RT_BASE_UI] = MeshBuilder::GenerateQuad("rt base ui", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_base_ui.png"), true);
		meshList[RT_PANEL_BASE] = MeshBuilder::GenerateQuad("rt panel base", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_panel_base.png"), true);
		meshList[RT_DISC] = MeshBuilder::GenerateQuad("rt disc", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_disc.png"), true);
		meshList[RT_OPU] = MeshBuilder::GenerateQuad("rt OPU", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_OPU.png"), true);
		meshList[RT_OPU_BASE] = MeshBuilder::GenerateQuad("rt OPU base", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_OPU_bg.png"), true);
		meshList[RT_UPLOAD_BTN] = MeshBuilder::GenerateQuad("rt upload", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_upload_btn.png"), true);
		meshList[RT_UPLOAD_BTN_BG] = MeshBuilder::GenerateQuad("rt upload bg", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_upload_btn_bg.png"), true);
		meshList[RT_LOSSLESS_BTN] = MeshBuilder::GenerateQuad("rt lossless", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_lossless_btn.png"), true);
		meshList[RT_LOSSLESS_BTN_BG] = MeshBuilder::GenerateQuad("rt lossless bg", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_lossless_btn_bg.png"), true);
		meshList[RT_COMPRESSED_BTN] = MeshBuilder::GenerateQuad("rt compressed", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_compressed_btn.png"), true);
		meshList[RT_COMPRESSED_BTN_BG] = MeshBuilder::GenerateQuad("rt compressed bg", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_compressed_btn_bg.png"), true);

		meshList[RHYTHM_BASE] = MeshBuilder::GenerateQuad("game base texture", vec3(), 1, 1, TextureLoader::LoadTexture("base_gradient.tga"));

		meshList[TRIGGER_BOX] = MeshBuilder::GenerateCube("trigger box", vec3(1), 1);
		meshList[INVISIBLE_WALL] = MeshBuilder::GenerateCube("invisible wall", vec3(1), 1);
	}

	// init roots
	{
		worldRoot = std::make_shared<RObj>();
		worldRoot->RootInit(RObj::WORLD, GROUP);

		viewRoot = std::make_shared<RObj>();
		viewRoot->RootInit(RObj::VIEW, GROUP);

		screenRoot = std::make_shared<RObj>();
		screenRoot->RootInit(RObj::SCREEN, GROUP);

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

			obj->AddPhysics(PhysicsObject::STATIC); // takes in PhysicsObject::BODY_TYPE
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(500, 0.5f, 500), vec3(0, -0.5f, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);

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

		RObj::setDefaultStat.Subscribe(BLACK, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});

		RObj::setDefaultStat.Subscribe(RT_BASE_UI, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->scl = vec3(2, 2, 1);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_PANEL_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->scl = vec3(2, 2, 1);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_DISC, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_OPU, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_OPU_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_UPLOAD_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_UPLOAD_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_LOSSLESS_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_LOSSLESS_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_COMPRESSED_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_COMPRESSED_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::NEON);
			obj->hasTransparency = true;
			});

		RObj::setDefaultStat.Subscribe(RHYTHM_BASE, [](const std::shared_ptr<RObj>& obj) {
			int height = 50;
			int width = 10;
			obj->offsetScl = vec3(width, height, 1);
			obj->offsetTrl = vec3(0, height * 0.5f, 0);

			obj->hasTransparency = true;
			});

		RObj::setDefaultStat.Subscribe(TRIGGER_BOX, [](const std::shared_ptr<RObj>& obj) {
			obj->allowRender = false;
			});
		RObj::setDefaultStat.Subscribe(INVISIBLE_WALL, [](const std::shared_ptr<RObj>& obj) {
			obj->allowRender = false;

			obj->AddPhysics(PhysicsObject::STATIC); // takes in PhysicsObject::BODY_TYPE
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(250, 0.5f, 0.5f), vec3(0, 0.5f, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(1.f);
			});
	}

	auto& newObj = RObj::newObject;
	PhysicsObject* physics = nullptr;
	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(AXES));

		worldRoot->NewChild(MeshObject::Create(GROUND));

		worldRoot->NewChild(MeshObject::Create(SKYBOX));

		float layerOffset = 0.035f;
		worldRoot->NewChild(MeshObject::Create(RT_BASE_UI));
		newObj->name = "rt base ui";
		newObj->trl = vec3(-30, 1.5f, 0);
		auto rtUI = newObj;
		uiPointsOfInterest[rtUI->name] = rtUI;

		rtUI->NewChild(MeshObject::Create(RT_UPLOAD_BTN));
		newObj->name = "upload btn";
		newObj->trl = vec3(0.225f, -0.165f, layerOffset * 2);
		uiPointsOfInterest[newObj->name] = newObj;
		newObj->NewChild(MeshObject::Create(RT_UPLOAD_BTN_BG));
		newObj->name = "upload bg";
		newObj->trl = vec3(0, 0, layerOffset * -0.25f);
		uiPointsOfInterest[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "upload trigger";
		newObj->AddPhysics(PhysicsObject::STATIC);
		physics = newObj->GetPhysics();
		physics->AddCollider(PhysicsObject::BOX, vec3(0.3f, 0.05f, 0.001f));
		physics->SetTrigger(true);
		triggerList[newObj->name] = newObj;

		rtUI->NewChild(MeshObject::Create(RT_LOSSLESS_BTN));
		newObj->name = "lossless btn";
		newObj->trl = vec3(-0.225f, -0.165f, layerOffset * 2);
		uiPointsOfInterest[newObj->name] = newObj;
		newObj->NewChild(MeshObject::Create(RT_LOSSLESS_BTN_BG));
		newObj->name = "lossless bg";
		newObj->trl = vec3(0, 0, layerOffset * -0.25f);
		uiPointsOfInterest[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "lossless trigger";
		newObj->AddPhysics(PhysicsObject::STATIC);
		physics = newObj->GetPhysics();
		physics->AddCollider(PhysicsObject::BOX, vec3(0.12f, 0.025f, 0.001f));
		physics->SetTrigger(true);
		triggerList[newObj->name] = newObj;

		rtUI->NewChild(MeshObject::Create(RT_COMPRESSED_BTN));
		newObj->name = "compressed btn";
		newObj->trl = vec3(-0.225f, -0.215f, layerOffset * 2);
		uiPointsOfInterest[newObj->name] = newObj;
		newObj->NewChild(MeshObject::Create(RT_COMPRESSED_BTN_BG));
		newObj->name = "compressed bg";
		newObj->trl = vec3(0, 0, layerOffset * -0.25f);
		uiPointsOfInterest[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "compressed trigger";
		newObj->AddPhysics(PhysicsObject::STATIC);
		physics = newObj->GetPhysics();
		physics->AddCollider(PhysicsObject::BOX, vec3(0.15f, 0.025f, 0.001f));
		physics->SetTrigger(true);
		triggerList[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(RT_PANEL_BASE));
		newObj->name = "panel ui";
		newObj->trl = vec3(30, 1.6f, layerOffset);
		auto panelUI = newObj;
		uiPointsOfInterest[panelUI->name] = panelUI;

		panelUI->NewChild(MeshObject::Create(RT_OPU_BASE));
		newObj->trl = vec3(-0.225f, 0, layerOffset);
		auto opuBase = newObj;
		opuBase->NewChild(MeshObject::Create(RT_DISC));
		newObj->trl = vec3(0, 0, layerOffset);
		opuBase->NewChild(MeshObject::Create(RT_OPU));
		newObj->trl = vec3(-0.0675f, 0.0675f, layerOffset * 2);

		worldRoot->NewChild(MeshObject::Create(INVISIBLE_WALL));
		physics = newObj->GetPhysics();
		physics->SetTransform(vec3(0, 0, 5), vec3());
		worldRoot->NewChild(MeshObject::Create(INVISIBLE_WALL));
		physics = newObj->GetPhysics();
		physics->SetTransform(vec3(5, 0, 0), vec3(0, 90, 0));
		worldRoot->NewChild(MeshObject::Create(INVISIBLE_WALL));
		physics = newObj->GetPhysics();
		physics->SetTransform(vec3(0, 0, -5), vec3());
		worldRoot->NewChild(MeshObject::Create(INVISIBLE_WALL));
		physics = newObj->GetPhysics();
		physics->SetTransform(vec3(-5, 0, 0), vec3(0, 90, 0));

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;
		}
	}

	// view space init
	{
		viewRoot->NewChild(MeshObject::Create(GROUP));
		auto rhythmGameGroup = newObj;
		newObj->name = "rhythm game";
		newObj->trl = vec3(0, -1, -2);
		newObj->rot = vec3(-75, 0, 0);
	}

	// screen space init
	{
		
		screenRoot->NewChild(TextObject::Create("dial_speaker", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0, -0.5f, 0);
		newObj->scl = vec3(30, 30, 1);
		screenRoot->NewChild(TextObject::Create("dial_text", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0, -0.575f, 0);
		newObj->scl = vec3(30, 30, 1);

		// debug text
		InitDebugText(FONT_CASCADIA_MONO); 
	}

	/************************ bellow for external class inits ************************/
	{
		// raycast init
		physicsRaycast.defaultRaycastEvent.Subscribe([&](const rp3d::RaycastInfo& rcInfo) {
			return PhysicsRaycast::STOP_ON_CONTACT;
			});

		// camera init
		camera.Init(glm::vec3(1, 1.5f, -1), vec3(0, 0, -1));
		camera.Set(FPCamera::MODE::FIRST_PERSON);
		camera.bobbingMaxPsi = 0.6f;
		camera.bobbingMaxX = 0.05f;
		camera.bobbingMaxY = 0.025f;

		// player init
		player.Init(worldRoot, GROUP, vec3(0, 0.5f, 0));
		player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0, 0, 4));
		player.speed = 75;
	}

	RObj::newObject.reset();
}

void SceneRhythm::Update(double dt) {
	BaseScene::Update(dt);
	ClearDebugText();

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

	// fps limitation + timer advancement
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

	switch (currentState) {
	case LOAD_IN:
		loadInTimer += dt;
		camera.SetDirection(vec3(0, 0, -1));

		if (loadInTimer > 0.2f) {
			currentState = START_INTERMISSION;
		}
		break;
	case START_INTERMISSION:
		dynamicIntermissionTimer += dt;
		if (dynamicIntermissionTimer > 3) {
			dynamicIntermissionTimer = 0;
			currentState = INTERMISSION;
		}
		break;
	case END_INTERMISSION:
		dynamicIntermissionTimer += dt;
		if (dynamicIntermissionTimer > 5) {
			dynamicIntermissionTimer = 0;
			currentState = START_GAME;
		}
		break;

	default: break;
	}

	auto& lightList = LightObject::lightList;
	auto& worldList = RObj::worldList;
	auto& viewList = RObj::viewList;
	auto& screenList = RObj::screenList;
	auto& physicsList = RObj::physicsList;

	
	HandleKeyPress();

	// player updates
	{
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
			obj->allowRender = !renderDebugPhysics;
			break;

		case GROUND:
			obj->allowRender = !renderDebugPhysics;
			break;

		default:
			break;
		}

		if (obj->name == "rt base ui") {
			static float accel = 0;
			switch (currentState) {
			case START_INTERMISSION:
				accel = 0;
				obj->trl = Smooth(obj->trl, vec3(0, 1.5f, 0), 25, dt);
				break;
			case INTERMISSION:
				break;
			case END_INTERMISSION:
				accel += 5 * dt;
				obj->trl.x += accel * dt;
				break;
			default: break;
			}
		}

		if (obj->name == "panel ui") {
			static float accel = 0;
			switch (currentState) {
			case START_INTERMISSION:
				accel = 0;
				obj->trl = Smooth(obj->trl, vec3(0, 1.6f, 0.035f), 25, dt);
				break;
			case INTERMISSION:
				break;
			case END_INTERMISSION:
				accel += 5 * dt;
				obj->trl.x -= accel * dt;
				break;
			default: break;
			}
		}

		if (debug) {

		}

		if (!obj->GetPhysics())
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

	// raycast
	if (currentState == INTERMISSION || currentState == RESULT) {
		physicsRaycast.ClearInfo();
		rp3d::Ray ray = MakeRay(camera.GetFinalPosition(), camera.GetFinalPosition() + camera.GetFinalDirection(), 2);
		PhysicsManager::GetInstance().GetWorld()->raycast(ray, &physicsRaycast);
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

			switch (currentState) {
			case START_INTERMISSION:
			case INTERMISSION:
			case END_INTERMISSION: {
				if (obj->name == "upload trigger") {
					auto uploadBg = uiPointsOfInterest["upload bg"].lock();
					physics->SetPosition(getPosFromModel(uploadBg->model));
					auto uploadBtn = uiPointsOfInterest["upload btn"].lock();

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							uploadBg->scl = Smooth(uploadBg->scl, vec3(0.9f), 5, dt);
							uploadBtn->scl = Smooth(uploadBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							uploadBg->scl = Smooth(uploadBg->scl, vec3(1), 10.f, dt);
							uploadBtn->scl = Smooth(uploadBtn->scl, vec3(1), 10.f, dt);
						}

						if (lmb_released) {
							currentState = END_INTERMISSION;
						}
					}
					else {
						uploadBg->scl = Smooth(uploadBg->scl, vec3(1, 0, 1), 10.f, dt);
						uploadBtn->scl = Smooth(uploadBtn->scl, vec3(1), 10.f, dt);
					}
				}
				if (obj->name == "lossless trigger") {
					auto losslessBg = uiPointsOfInterest["lossless bg"].lock();
					physics->SetPosition(getPosFromModel(losslessBg->model));
					auto losslessBtn = uiPointsOfInterest["lossless btn"].lock();

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							losslessBg->scl = Smooth(losslessBg->scl, vec3(0.9f), 5, dt);
							losslessBtn->scl = Smooth(losslessBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							losslessBg->scl = Smooth(losslessBg->scl, vec3(1), 10.f, dt);
							losslessBtn->scl = Smooth(losslessBtn->scl, vec3(1), 10.f, dt);
						}

						if (lmb_released) {
							difficulty = 1;
						}
					}
					else {
						losslessBg->scl = Smooth(losslessBg->scl, vec3(1, 0, 1), 10.f, dt);
						losslessBtn->scl = Smooth(losslessBtn->scl, vec3(1), 10.f, dt);
					}
				}
				if (obj->name == "compressed trigger") {
					auto compresseddBg = uiPointsOfInterest["compressed bg"].lock();
					physics->SetPosition(getPosFromModel(compresseddBg->model));
					auto compressedBtn = uiPointsOfInterest["compressed btn"].lock();

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							compresseddBg->scl = Smooth(compresseddBg->scl, vec3(0.9f), 5, dt);
							compressedBtn->scl = Smooth(compressedBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							compresseddBg->scl = Smooth(compresseddBg->scl, vec3(1), 10.f, dt);
							compressedBtn->scl = Smooth(compressedBtn->scl, vec3(1), 10.f, dt);
						}

						if (lmb_released) {
							difficulty = 0;
						}
					}
					else {
						compresseddBg->scl = Smooth(compresseddBg->scl, vec3(1, 0, 1), 10.f, dt);
						compressedBtn->scl = Smooth(compressedBtn->scl, vec3(1), 10.f, dt);
					}
				}
				break;
			}
			case GAME: {

				break;
			}
			case START_RESULT:
			case RESULT:
			case END_RESULT: {

				break;
			}
			default: break;
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

	// clean world list if dirty
	if (dirtyWorldList) {
		for (unsigned i = 0; i < worldList.size(); ) {
			if (worldList[i].expired()) {
				worldList.erase(worldList.begin() + i);
				continue;
			}
			i++;
		}
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

	// render debug physics
	if (ALLOW_PHYSICS_DEBUG && renderDebugPhysics && debugPhysicsWorld) {
		modelStack.Clear();
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform1i(m_parameters[U_LIGHT_ENABLED], 0);

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

void SceneRhythm::Exit() {
	BaseScene::Exit();


}

void SceneRhythm::HandleKeyPress() {


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

	// dialogue controls
	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE)) {
		if (DialogueManager::GetInstance().CheckActivePack()) {
			DialogueManager::GetInstance().ControlCurrentDialogue();
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

			lmb_pressed = lmb_down = lmb_released = rmb_pressed = rmb_down = rmb_released = false;
			if (MouseController::GetInstance()->IsButtonPressed(MouseController::LMB)) {
				lmb_pressed = true;
			}
			if (MouseController::GetInstance()->IsButtonDown(MouseController::LMB)) {
				lmb_down = true;
			}
			if (MouseController::GetInstance()->IsButtonReleased(MouseController::LMB)) {
				lmb_released = true;
			}
			if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB)) {
				rmb_pressed = true;
			}
			if (MouseController::GetInstance()->IsButtonDown(MouseController::RMB)) {
				rmb_down = true;
			}
			if (MouseController::GetInstance()->IsButtonReleased(MouseController::RMB)) {
				rmb_released = true;
			}
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
