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
#include "RhythmGameManager.h"

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

	glClearColor(1, 1, 1, 0.0f);

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
		RhythmGameManager::GetInstance().SetDirectory("SceneRhythm/Chart");
	}

	// audio init
	{
		// sfx init
		AudioManager::GetInstance().LoadSFX(GOOFY_AHH_ASRIEL_STAR_SOUND, "sfx_asriel_star_drop.wav");

	}

	// Init VBO here
	{
		for (int i = 0; i < static_cast<int>(TOTAL); ++i)
		{
			meshList[i] = nullptr;
		}

		meshList[AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5);
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox");
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));

		meshList[BLACK] = MeshBuilder::GenerateQuad("black", vec3(), 1, 1, TextureLoader::LoadTexture("black.png"));

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

		meshList[RHYTHM_BASE] = MeshBuilder::GenerateQuad("game base", vec3(), 1, 1, TextureLoader::LoadTexture("base_gradient.tga"));
		meshList[RHYTHM_BEAT] = MeshBuilder::GenerateQuad("beat", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_beat.png"));
		meshList[RHYTHM_HIT_POINT] = MeshBuilder::GenerateQuad("hit point", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_hit_point.png"));
		meshList[RHYTHM_TAP_NOTE] = MeshBuilder::GenerateQuad("tap note", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_tap_note.png"));
		meshList[RHYTHM_HOLD_NOTE] = MeshBuilder::GenerateQuad("hold note", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_hold_note.png"));
		meshList[RHYTHM_HOLD_NOTE_BODY] = MeshBuilder::GenerateCube("hold note body", vec3(1), 1);

		meshList[VFX_TAP_NOTE] = MeshBuilder::GenerateQuad("tap note", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_tap_note.png"));
		meshList[VFX_HOLD_NOTE] = MeshBuilder::GenerateQuad("hold note", vec3(), 1, 1, TextureLoader::LoadTexture("GAME_hold_note.png"));
		meshList[VFX_HOLD_NOTE_BODY] = MeshBuilder::GenerateCube("hold note body", vec3(1), 1);

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
			obj->material.Set(vec3(0.6f), vec3(0.65f), vec3(0), 1);
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
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(FONT_CASCADIA_MONO, [](const std::shared_ptr<RObj>& obj) {
			});

		RObj::setDefaultStat.Subscribe(BLACK, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			});

		RObj::setDefaultStat.Subscribe(RT_BASE_UI, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->scl = vec3(2, 2, 1);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_PANEL_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->scl = vec3(2, 2, 1);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_DISC, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_OPU, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_OPU_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_UPLOAD_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_UPLOAD_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_LOSSLESS_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_LOSSLESS_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_COMPRESSED_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_COMPRESSED_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});

		RObj::setDefaultStat.Subscribe(RHYTHM_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			int height = 15;
			int width = 4;
			obj->offsetScl = vec3(width, height, 1);
			obj->offsetTrl = vec3(0, height * 0.5f, 0);

			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RHYTHM_BEAT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->rot = vec3(-90, 0, 0);
			obj->offsetTrl = vec3(0, 0, -0.5f);

			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RHYTHM_HIT_POINT, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.65f);

			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RHYTHM_TAP_NOTE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.65f);

			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RHYTHM_HOLD_NOTE, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.65f);

			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RHYTHM_HOLD_NOTE_BODY, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetTrl = vec3(0, 0, -0.5f);
			obj->offsetScl = vec3(0.075f, 0.075f, 1);

			obj->hasTransparency = true;
			});

		RObj::setDefaultStat.Subscribe(VFX_TAP_NOTE, [&](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.65f);

			obj->hasTransparency = true;
			VFXList.push_back(obj);
			});
		RObj::setDefaultStat.Subscribe(VFX_HOLD_NOTE, [&](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.65f);

			obj->hasTransparency = true;
			VFXList.push_back(obj);
			});
		RObj::setDefaultStat.Subscribe(VFX_HOLD_NOTE_BODY, [&](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetTrl = vec3(0, 0, -0.5f);
			obj->offsetScl = vec3(0.075f, 0.075f, 1);
			obj->scl = vec3(1, 1, 0.01f);

			obj->hasTransparency = true;
			VFXList.push_back(obj);
			});

		RObj::setDefaultStat.Subscribe(TRIGGER_BOX, [](const std::shared_ptr<RObj>& obj) {
			obj->allowRender = false;
			});
		RObj::setDestroyedEvent.Subscribe(TRIGGER_BOX, [&](const std::shared_ptr<RObj>& obj) {
			dirtyWorldList = true;
			});
		RObj::setDefaultStat.Subscribe(INVISIBLE_WALL, [](const std::shared_ptr<RObj>& obj) {
			obj->allowRender = false;

			obj->AddPhysics(PhysicsObject::STATIC); // takes in PhysicsObject::BODY_TYPE
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(250, 0.5f, 0.5f), vec3(0, 0.5f, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(1.f);
			});
		RObj::setDestroyedEvent.Subscribe(INVISIBLE_WALL, [&](const std::shared_ptr<RObj>& obj) {
			dirtyWorldList = true;
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

		worldRoot->NewChild(MeshObject::Create(GROUP));
		newObj->name = "rhythm base";
		newObj->trl = vec3(0, 0.4999f, 1);
		newObj->rot = vec3(-90, 0, 0);
		auto rhythmBase = newObj;
		inGamePointsOfInterest[newObj->name] = newObj;

		rhythmBase->NewChild(MeshObject::Create(RHYTHM_BASE));

		worldRoot->NewChild(MeshObject::Create(RHYTHM_HIT_POINT));
		newObj->name = "lane 0 point";
		inGamePointsOfInterest[newObj->name] = newObj;
		worldRoot->NewChild(MeshObject::Create(RHYTHM_HIT_POINT));
		newObj->name = "lane 1 point";
		inGamePointsOfInterest[newObj->name] = newObj;
		worldRoot->NewChild(MeshObject::Create(RHYTHM_HIT_POINT));
		newObj->name = "lane 2 point";
		inGamePointsOfInterest[newObj->name] = newObj;
		worldRoot->NewChild(MeshObject::Create(RHYTHM_HIT_POINT));
		newObj->name = "lane 3 point";
		inGamePointsOfInterest[newObj->name] = newObj;

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;

			rtUI->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				auto& lp = newLightObj->lightProperties;
				lp.type = Light::LIGHT_POINT;
				lp.color = vec3(0.92f, 0.95f, 1);
				lp.power = 1;
				lp.kC = 0.75f;
				lp.kL = 0.005f;
				lp.kQ = 0.05f;

				UpdateLightUniform(newLightObj);
			}

			panelUI->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				auto& lp = newLightObj->lightProperties;
				lp.type = Light::LIGHT_POINT;
				lp.color = vec3(0.92f, 0.95f, 1);
				lp.power = 0.75f;
				lp.kC = 0.5f;
				lp.kL = 0.01f;
				lp.kQ = 0.1f;

				UpdateLightUniform(newLightObj);
			}

			rhythmBase->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				auto& lp = newLightObj->lightProperties;
				newLightObj->trl = vec3(0, 1.5f, 1);
				newLightObj->hasTransparency = true;
				lp.type = Light::LIGHT_POINT;
				lp.color = vec3(0.92f, 0.95f, 1);
				lp.power = 1;
				lp.kC = 1;
				lp.kL = 0.005f;
				lp.kQ = 0.001f;

				UpdateLightUniform(newLightObj);
			}
			rhythmBase->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				auto& lp = newLightObj->lightProperties;
				newLightObj->trl = vec3(0, 10, 1);
				newLightObj->hasTransparency = true;
				lp.type = Light::LIGHT_POINT;
				lp.color = vec3(0.92f, 0.95f, 1);
				lp.power = 1;
				lp.kC = 1;
				lp.kL = 0.005f;
				lp.kQ = 0.001f;

				UpdateLightUniform(newLightObj);
			}
		}
	}

	// view space init
	{
		
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
		camera.Set(FPCamera::MODE::LOCK_ON);
		camera.bobbingMaxPsi = 0.6f;
		camera.bobbingMaxX = 0.05f;
		camera.bobbingMaxY = 0.025f;

		// player init
		player.Init(worldRoot, GROUP, vec3(0, 0.5f, 0));
		player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0, 0, 4));
		player.speed = 75;
	}

	RObj::newObject.reset();

	// atmosphere init
	{
		atmosphere.Set(vec3(0.09f, 0.12f, 0.2f), 1, 0.01f, 1, 20);
		atmosphereTargetingDensestRange = 10;
		UpdateAtmosphereUniform();
	}

	// game info init
	{
		globalSurroundingColor = vec3(0.09f, 0.12f, 0.2f);
		RhythmGameManager::GetInstance().SetDetectRange(std::array<float, RhythmGameManager::TOTAL_SCORE_TYPE>({ 0.15f, 0.1f, 0.625f, 0.025f}));
		RhythmBeat::createEvent.Subscribe([&](RhythmBeat* beat) {
			worldRoot->NewChild(MeshObject::Create(RHYTHM_BEAT));
			auto& newObj = RObj::newObject;
			beat->render = newObj;

			newObj.reset();
			});
		auto ColorByLane = [](int laneIndex) {
			switch (laneIndex) {
			case 0: return vec3(1.f, 0.3f, 0.3f);
			case 1: return vec3(1.f, 1.f, 0.3f);
			case 2: return vec3(0.3f, 1.f, 1.f);
			case 3: return vec3(0.3f, 1.f, 0.3f);
			default: return vec3(1);
			}
			};
		for (auto& pair : inGamePointsOfInterest) {
			auto obj = pair.second.lock();

			if (pair.first.find("lane") != std::string::npos) {
				int index = std::stoi(pair.first.substr(5, 1));
				obj->colorFilter = ColorByLane(index);
			}
		}
		RhythmNote::activeEvent.Subscribe([&](RhythmNote* note) {

			if (note->type == RhythmNote::TAP) {
				auto tapNote = static_cast<TapNote*>(note);
				auto& newObj = RObj::newObject;
				worldRoot->NewChild(MeshObject::Create(RHYTHM_TAP_NOTE));
				tapNote->render = newObj;
				newObj->colorFilter = ColorByLane(tapNote->lane);

				newObj.reset();
			}
			else if (note->type == RhythmNote::HOLD) {
				auto holdNote = static_cast<HoldNote*>(note);
				auto& newObj = RObj::newObject;
				worldRoot->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE));
				holdNote->startRender = newObj;
				newObj->colorFilter = ColorByLane(holdNote->lane);

				worldRoot->NewChild(MeshObject::Create(GROUP));
				holdNote->lengthRender = newObj;
				newObj->colorFilter = ColorByLane(holdNote->lane);
				auto holdNoteGroup = newObj;

				holdNoteGroup->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE_BODY));
				newObj->offsetTrl += vec3(0, 0.075f, 0);
				holdNoteGroup->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE_BODY));
				newObj->offsetTrl += vec3(0.075f, 0, 0);
				holdNoteGroup->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE_BODY));
				newObj->offsetTrl += vec3(0, -0.075f, 0);
				holdNoteGroup->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE_BODY));
				newObj->offsetTrl += vec3(-0.075f, 0, 0);

				worldRoot->NewChild(MeshObject::Create(RHYTHM_HOLD_NOTE));
				holdNote->endRender = newObj;
				newObj->colorFilter = ColorByLane(holdNote->lane);

				newObj.reset();
			}

			});
		RhythmNote::hitEvent.Subscribe([&](RhythmNote* note) {

			if (note->type == RhythmNote::TAP) {
				auto tapNote = static_cast<TapNote*>(note);
				auto& newObj = RObj::newObject;
				worldRoot->NewChild(MeshObject::Create(VFX_TAP_NOTE));
				newObj->trl = tapNote->render.lock()->trl;
				newObj->colorFilter = ColorByLane(tapNote->lane);

				newObj.reset();
			}
			else if (note->type == RhythmNote::HOLD) {
				auto holdNote = static_cast<HoldNote*>(note);
				auto& newObj = RObj::newObject;
				const float HOLD_VFX_CD = 0.1f;

				if (!holdNote->startRender.expired()) {
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE));
					newObj->trl = holdNote->startRender.lock()->trl;
					newObj->colorFilter = ColorByLane(holdNote->lane);
				}
				else if (holdNote->holding) {
					if (holdNote->holdTimer < 0.1f)
						return;
					holdNote->holdTimer -= 0.1f;

					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = getPosFromModel(holdNote->lengthRender.lock()->children[0]->model);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = getPosFromModel(holdNote->lengthRender.lock()->children[1]->model);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = getPosFromModel(holdNote->lengthRender.lock()->children[2]->model);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = getPosFromModel(holdNote->lengthRender.lock()->children[3]->model);
					newObj->colorFilter = ColorByLane(holdNote->lane);
				}
				else {
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE));
					newObj->trl = holdNote->endRender.lock()->trl;
					newObj->colorFilter = ColorByLane(holdNote->lane);
				}

				newObj.reset();
			}

			});
		auto& lanes = RhythmGameManager::GetInstance().GetLanes();
		float xPos = -1.5f;
		for (auto& lane : lanes) {
			lane.SetLane(vec3(xPos, 1, 0), vec3(0, 0, -1), 10, -0.5f, -0.1f);
			xPos += 1;
		}
	}
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

	// atmosphere
	atmosphere.color = Smooth(atmosphere.color, globalSurroundingColor, 20, dt);
	UpdateAtmosphereUniform(U_ATMOSPHERE_COLOR);
	atmosphere.densestRange = Smooth(atmosphere.densestRange, atmosphereTargetingDensestRange, 40, dt);
	UpdateAtmosphereUniform(U_ATMOSPHERE_DENSEST_RANGE);

	// state switch
	dynamicStateTimer += dt;
	switch (currentState) {
	case LOAD_IN:
		camera.SetDirection(vec3(0, 0, -1));

		if (dynamicStateTimer > 0.2f) {
			dynamicStateTimer = 0;
			camera.Set(FPCamera::MODE::FIRST_PERSON);
			currentState = START_INTERMISSION;
		}
		break;

	case START_INTERMISSION:
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;
			currentState = INTERMISSION;
		}
		break;

	case INTERMISSION:
		dynamicStateTimer = 0;
		break;

	case END_INTERMISSION:
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;
			currentState = START_GAME;
			if (difficulty == 0)
				RhythmGameManager::GetInstance().LoadGame("A_Cybers_World_Compressed.json");
			else 
				RhythmGameManager::GetInstance().LoadGame("A_Cybers_World_Lossless.json");
		}
		break;

	case START_GAME:
		atmosphereTargetingDensestRange = 10;
		
		player.allowControl = false;
		camera.Set(Cam::MODE::LOCKED);
		camera.SetDirection(vec3(0, -1, -3));
		camera.basePosition = vec3(0, 2.5f, 2);

		if (dynamicStateTimer > 1.5f) {
			dynamicStateTimer = 0;
			currentState = GAME;
			RhythmGameManager::GetInstance().StartGame();
		}
		break;

	case GAME:
		player.allowControl = false;
		camera.Set(Cam::MODE::LOCKED);
		camera.SetDirection(vec3(0, -1, -3));
		camera.basePosition = vec3(0, 2.5f, 2);

		dynamicStateTimer = 0;
		break;

	case START_RESULT:
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;
			currentState = RESULT;
		}
		break;

	case RESULT:
		dynamicStateTimer = 0;
		break;

	case END_RESULT:
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;
			currentState = START_INTERMISSION;
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

	// rhythm game update
	RhythmGameManager::GetInstance().Update(dt);

	// player updates
	{
		// update position and camera bobbing
		if (camera.GetCurrentMode() != Cam::MODE::FREE && camera.GetCurrentMode() != Cam::MODE::LOCKED)
			player.UpdatePhysicsWithCamera(dt, camera);
		else
			player.UpdatePhysics(dt);
	}

	// in game poi
	for (auto& pair : inGamePointsOfInterest) {
		auto obj = pair.second.lock();

		if (pair.first.find("lane") != std::string::npos) {
			int index = std::stoi(pair.first.substr(5, 1));
			obj->trl = RhythmGameManager::GetInstance().GetLanes()[index].position;
		}

		if (currentState == START_GAME) {
			obj->allowRender = true;
			obj->alpha = Smooth(obj->alpha, 1.f, 30, dt);
		}
		else if (currentState == GAME) {

		}
		else if (currentState == START_RESULT) {
			obj->alpha = Smooth(obj->alpha, 0.f, 20, dt);
		}
		else {
			obj->alpha = 0.f;
		}
	}

	// vfx list
	for (unsigned i = 0; i < VFXList.size(); ) {
		if (VFXList[i].expired()) {
			VFXList.erase(VFXList.begin() + i);
			continue;
		}
		auto obj = VFXList[i].lock();

		switch (static_cast<GEOMETRY_TYPE>(obj->geometryType)) {
		case VFX_TAP_NOTE:
		case VFX_HOLD_NOTE:
		case VFX_HOLD_NOTE_BODY:
			obj->scl = Smooth(obj->scl, vec3(2, 2, 0.01f), 10, dt);
			obj->alpha = Smooth(obj->alpha, 0.f, 10, dt);

			if (obj->alpha <= 0.001f) {
				obj->Destroy();
			}
			break;

		default: break;
		}

		i++;
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
			obj->colorFilter = Smooth(obj->colorFilter, globalSurroundingColor, 20, dt);
			break;

		case GROUND:
			obj->allowRender = !renderDebugPhysics;
			obj->colorFilter = Smooth(obj->colorFilter, globalSurroundingColor, 20, dt);
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

		float savedPower = properties.power;
		properties.power *= obj->alpha;

		UpdateLightUniform(obj);
		properties.power = savedPower;

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
	AddDebugText("RhythmGameManager.maxDisplayBeat: " + std::to_string(RhythmGameManager::GetInstance().maxDisplayBeat));
	AddDebugText("RhythmGameManager.prevMaxDisplayBeat_int: " + std::to_string(RhythmGameManager::GetInstance().prevMaxDisplayBeat_int));
	AddDebugText("RhythmGameManager.currentBeat: " + std::to_string(RhythmGameManager::GetInstance().currentBeat));
	AddDebugText("RhythmGameManager.lastestScoreType: " + std::to_string(RhythmGameManager::GetInstance().GetScoreType()));

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

	if (currentState == GAME) {
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F))
			RhythmGameManager::GetInstance().TappedLane(0);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_F))
			RhythmGameManager::GetInstance().HeldLane(0);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_G))
			RhythmGameManager::GetInstance().TappedLane(1);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_G))
			RhythmGameManager::GetInstance().HeldLane(1);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_H))
			RhythmGameManager::GetInstance().TappedLane(2);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_H))
			RhythmGameManager::GetInstance().HeldLane(2);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_J))
			RhythmGameManager::GetInstance().TappedLane(3);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J))
			RhythmGameManager::GetInstance().HeldLane(3);
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

	glUniform3fv(m_parameters[U_COLOR_FILTER], 1, &obj->accumulatedColorFilter.r);
	glUniform1f(m_parameters[U_COLOR_ALPHA], obj->accumulatedAlpha);

	if (obj->geometryType == GROUP) {
		glUniform1f(m_parameters[U_COLOR_ALPHA], 0.1f);
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
