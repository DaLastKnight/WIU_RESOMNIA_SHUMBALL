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

#include "SceneMedical.h"

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
#include "SceneManager.h"

#include "Console.h"
#include "Utils.h"
#include "Ease.h"

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
		AudioManager::GetInstance().VolumeMUS(1);
		AudioManager::GetInstance().VolumeChannel(-1, 1);

		// music init (handled in RhythmGameManager)

		// sfx init
		AudioManager::GetInstance().LoadSFX(SFX_CLICK, "click.mp3");
		AudioManager::GetInstance().LoadSFX(SFX_FAIL, "fail.mp3");
		AudioManager::GetInstance().VolumeSFX(SFX_FAIL, 0.5f);
		AudioManager::GetInstance().LoadSFX(SFX_TADA, "tada.mp3");
		AudioManager::GetInstance().VolumeSFX(SFX_TADA, 0.75f);
		AudioManager::GetInstance().LoadSFX(SFX_SWOOSH, "swoosh.mp3");
		AudioManager::GetInstance().LoadSFX(SFX_TICK, "tick.wav");
		AudioManager::GetInstance().VolumeSFX(SFX_TICK, 0.5f);
		AudioManager::GetInstance().LoadSFX(SFX_LANE0, "lane0.wav");
		AudioManager::GetInstance().LoadSFX(SFX_LANE1, "lane1.wav");
		AudioManager::GetInstance().LoadSFX(SFX_LANE2, "lane2.wav");
		AudioManager::GetInstance().LoadSFX(SFX_LANE3, "lane3.wav");

	}

	// dialogue init
	{
		DialogueManager::GetInstance().LoadDialoguePack("Uploading.json");
		DialogueManager::GetInstance().LoadDialoguePack("Resending_Packet.json");
		DialogueManager::GetInstance().LoadDialoguePack("System_Timeout.json");
		DialogueManager::GetInstance().LoadDialoguePack("Upload_Succesful.json");
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
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1), 0.05f);
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1), 0.05f);

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));

		meshList[BLACK] = MeshBuilder::GenerateQuad("black", vec3(), 1, 1, TextureLoader::LoadTexture("black.png"));

		baseUITexture[0] = TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_base_ui.png");
		baseUITexture[1] = TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_result.png");
		meshList[RT_BASE_UI] = MeshBuilder::GenerateQuad("rt base ui", vec3(), 1, 1, baseUITexture[0], true);
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
		meshList[RT_NEXT_BTN] = MeshBuilder::GenerateQuad("rt next", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_next_btn.png"), true);
		meshList[RT_NEXT_BTN_BG] = MeshBuilder::GenerateQuad("rt next bg", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_next_btn_bg.png"), true);
		meshList[RT_RETRY_BTN] = MeshBuilder::GenerateQuad("rt retry", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_retry_btn.png"), true);
		meshList[RT_RETRY_BTN_BG] = MeshBuilder::GenerateQuad("rt retry bg", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_retry_btn_bg.png"), true);
		meshList[RT_PROGRESSION] = MeshBuilder::GenerateQuad("rt progression", vec3(), 1, 1, TextureLoader::LoadTexture("RHYTHM_TRANSFER_EXE_progression_.png"));
		meshList[RT_PROGRESS] = MeshBuilder::GenerateQuad("rt progress", vec3(1), 1, 1);
		meshList[RT_PROGRESS_INDICATOR] = MeshBuilder::GenerateQuad("rt progress indicator", vec3(1), 1, 1);

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
		RObj::setDefaultStat.Subscribe(RT_NEXT_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_NEXT_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_RETRY_BTN, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_RETRY_BTN_BG, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_PROGRESSION, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_PROGRESS, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(1, 0.05f, 1);
			obj->offsetTrl.x = 0.5f;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(RT_PROGRESS_INDICATOR, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::BRIGHT);
			obj->offsetScl = vec3(0.01f, 0.05f, 1);
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

		rtUI->NewChild(MeshObject::Create(RT_NEXT_BTN));
		newObj->name = "next btn";
		newObj->trl = vec3(0.23f, -0.19f, layerOffset * 2);
		uiPointsOfInterest[newObj->name] = newObj;
		newObj->NewChild(MeshObject::Create(RT_NEXT_BTN_BG));
		newObj->name = "next bg";
		newObj->trl = vec3(0, 0, layerOffset * -0.25f);
		uiPointsOfInterest[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "next trigger";
		newObj->AddPhysics(PhysicsObject::STATIC);
		physics = newObj->GetPhysics();
		physics->AddCollider(PhysicsObject::BOX, vec3(0.2f, 0.05f, 0.001f));
		physics->SetTrigger(true);
		triggerList[newObj->name] = newObj;

		rtUI->NewChild(MeshObject::Create(RT_RETRY_BTN));
		newObj->name = "retry btn";
		newObj->trl = vec3(-0.23f, -0.19f, layerOffset * 2);
		uiPointsOfInterest[newObj->name] = newObj;
		newObj->NewChild(MeshObject::Create(RT_RETRY_BTN_BG));
		newObj->name = "retry bg";
		newObj->trl = vec3(0, 0, layerOffset * -0.25f);
		uiPointsOfInterest[newObj->name] = newObj;

		worldRoot->NewChild(MeshObject::Create(TRIGGER_BOX));
		newObj->name = "retry trigger";
		newObj->AddPhysics(PhysicsObject::STATIC);
		physics = newObj->GetPhysics();
		physics->AddCollider(PhysicsObject::BOX, vec3(0.2f, 0.05f, 0.001f));
		physics->SetTrigger(true);
		triggerList[newObj->name] = newObj;

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
		newObj->name = "disc";
		newObj->trl = vec3(0, 0, layerOffset);
		uiPointsOfInterest[newObj->name] = newObj;
		opuBase->NewChild(MeshObject::Create(RT_OPU));
		newObj->trl = vec3(-0.0675f, 0.0675f, layerOffset * 2);

		worldRoot->NewChild(MeshObject::Create(RT_PROGRESSION));
		newObj->name = "progression";
		newObj->trl = vec3(0, 0, layerOffset);
		auto progression = newObj;
		inGamePointsOfInterest[progression->name] = progression;

		progression->NewChild(MeshObject::Create(GROUP));
		newObj->name = "progress bar";
		newObj->trl = vec3(0, -0.1f, 0);
		auto progressBar = newObj;
		inGamePointsOfInterest[progressBar->name] = progressBar;
		progressBar->NewChild(MeshObject::Create(RT_PROGRESS_INDICATOR));
		newObj->trl = vec3(-0.505f, 0, 0.005f);
		progressBar->NewChild(MeshObject::Create(RT_PROGRESS_INDICATOR));
		newObj->trl = vec3(0.505f, 0, 0.005f);

		progression->NewChild(TextObject::Create("dial_text", "test test", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(0, -0.2f, 0);
		newObj->offsetScl = vec3(0.1f, 0.1f, 1);
		newObj->colorFilter = vec3(0.004f, 0.337f, 1);
		newObj->alpha = 0.5f;

		progression->NewChild(TextObject::Create("scores", "score: ", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(0, -0.3f, 0);
		newObj->offsetScl = vec3(0.075f, 0.075f, 1);
		newObj->colorFilter = vec3(0.004f, 0.337f, 1);
		newObj->alpha = 0.5f;

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

		progression->NewChild(TextObject::Create("high score", "", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->trl.y = -0.375f;
		newObj->offsetTrl.z = 0.025f;
		newObj->offsetScl = vec3(0.075f, 0.075f, 1);
		newObj->colorFilter = vec3(0.004f, 0.337f, 1);
		newObj->alpha = 0.5f;
		uiPointsOfInterest[newObj->name] = newObj;

		progression->NewChild(TextObject::Create("new high score", "", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->trl.y = -0.45f;
		newObj->offsetTrl.z = 0.05f;
		newObj->offsetScl = vec3(0.075f, 0.075f, 1);
		newObj->colorFilter = vec3(1, 1, 0.337f);
		newObj->alpha = 0.5f;
		uiPointsOfInterest[newObj->name] = newObj;

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
		
		screenRoot->NewChild(TextObject::Create("controls", "", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(-0.98f, -0.95f, 0);
		newObj->scl = vec3(30, 30, 1);
		auto newTextObj = std::static_pointer_cast<TextObject>(newObj);
		newTextObj->centerText = false;
		newTextObj->text = "controls: F | G | H | J";

		screenRoot->NewChild(TextObject::Create("comment", "", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0, -0.75f, 0);
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
			case 0: return vec3(1.f, 0.5f, 0.5f);
			case 1: return vec3(1.f, 1.f, 0.5f);
			case 2: return vec3(0.5f, 1.f, 1.f);
			case 3: return vec3(0.5f, 1.f, 0.5f);
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

			auto SFXFromLane = [](int laneIndex) {
				switch (laneIndex) {
				case 0: return SFX_LANE0;
				case 1: return SFX_LANE1;
				case 2: return SFX_LANE2;
				case 3: return SFX_LANE3;
				default: return SFX_LANE0;
				}
				};

			if (note->type == RhythmNote::TAP) {
				auto tapNote = static_cast<TapNote*>(note);
				auto& newObj = RObj::newObject;
				worldRoot->NewChild(MeshObject::Create(VFX_TAP_NOTE));
				newObj->trl = tapNote->render.lock()->trl;
				newObj->colorFilter = ColorByLane(tapNote->lane);

				newObj.reset();

				AudioManager::GetInstance().PlaySFX(SFXFromLane(tapNote->lane));
			}
			else if (note->type == RhythmNote::HOLD) {
				auto holdNote = static_cast<HoldNote*>(note);
				auto& newObj = RObj::newObject;
				const float HOLD_VFX_CD = 0.1f;

				if (!holdNote->startRender.expired()) {
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE));
					newObj->trl = holdNote->startRender.lock()->trl;
					newObj->colorFilter = ColorByLane(holdNote->lane);

					AudioManager::GetInstance().PlaySFX(SFXFromLane(holdNote->lane));
				}
				else if (holdNote->holding) {
					if (holdNote->holdTimer < 0.1f)
						return;
					holdNote->holdTimer -= 0.1f;

					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = holdNote->lengthRender.lock()->trl;
					newObj->offsetTrl += vec3(0, 0.075f, 0);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = holdNote->lengthRender.lock()->trl;
					newObj->offsetTrl += vec3(0.075f, 0, 0);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = holdNote->lengthRender.lock()->trl;
					newObj->offsetTrl += vec3(0, -0.075f, 0);
					newObj->colorFilter = ColorByLane(holdNote->lane);
					worldRoot->NewChild(MeshObject::Create(VFX_HOLD_NOTE_BODY));
					newObj->trl = holdNote->lengthRender.lock()->trl;
					newObj->offsetTrl += vec3(-0.075f, 0, 0);
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
			lane.SetLane(vec3(xPos, 1, 0), vec3(0, 0, -1), 12.5, -0.5f, -0.1f);
			xPos += 1;
		}
		RhythmGameManager::GetInstance().SetTickSFXKey(SFX_TICK);
		RhythmGameManager::GetInstance().SetAutoPlay(false);

		// scene collab
		collabScore = DataManager::GetInstance().GetCurrentData(DataManager::COLLAB_SCORE);
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

	AddDebugText("currentState: " + std::to_string(currentState));

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
		if (dynamicStateTimer > 2) {
			dynamicStateTimer = 0;
			currentState = INTERMISSION;
		}
		break;

	case INTERMISSION:
		dynamicStateTimer = 0;
		situationTextColor = vec3(0.004f, 0.337f, 1);
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
		player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0, 0, 2));
		camera.Set(Cam::MODE::LOCKED);
		camera.SetDirection(vec3(0, -1, -3));
		camera.basePosition = vec3(0, 2.5f, 2);

		if (dynamicStateTimer > 1.5f) {
			dynamicStateTimer = 0;
			currentState = GAME;
			RhythmGameManager::GetInstance().StartGame();

			allowActivatePacketLoss = allowActivateUploadSuccessful = true;
			DialogueManager::GetInstance().StartDialogue("Uploading");
		}
		break;

	case GAME:
		player.allowControl = false;
		camera.Set(Cam::MODE::LOCKED);
		camera.SetDirection(vec3(0, -1, -3));
		camera.basePosition = vec3(0, 2.5f, 2);

		if (RhythmGameManager::GetInstance().GetProgressionMaxed()) {
			if (allowActivatePacketLoss) {
				DialogueManager::GetInstance().EndDialogue();
				DialogueManager::GetInstance().StartDialogue("Resending_Packet");
				allowActivatePacketLoss = false;
			}
			else if (allowActivateUploadSuccessful && RhythmGameManager::GetInstance().GetProgressionFixed()) {
				DialogueManager::GetInstance().EndDialogue();
				DialogueManager::GetInstance().StartDialogue("Upload_Succesful");
				situationTextColor = vec3(0.004f, 1, 0.337f);
				allowActivateUploadSuccessful = false;
			}
		}

		if (!RhythmGameManager::GetInstance().CheckMusicPlaying() || forceEndGame) {
			AudioManager::GetInstance().FadeOutMUS(2750);
			currentState = START_RESULT;
			camera.basePosition = player.position + player.cameraOffset;
			camera.SetDirection(vec3(0, 0, -1));
		}

		dynamicStateTimer = 0;
		break;

	case START_RESULT:
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;
			
			if (!RhythmGameManager::GetInstance().GetProgressionFixed() || !RhythmGameManager::GetInstance().GetProgressionMaxed()) {
				DialogueManager::GetInstance().EndDialogue();
				DialogueManager::GetInstance().StartDialogue("System_Timeout");
				situationTextColor = vec3(1, 0.004f, 0.337f);
			}
			else {
				DialogueManager::GetInstance().EndDialogue();
				DialogueManager::GetInstance().StartDialogue("Upload_Succesful");
				situationTextColor = vec3(0.004f, 1, 0.337f);
			}
			
			AudioManager::GetInstance().PauseMUS();
			RhythmGameManager::GetInstance().EndGame();

			player.allowControl = true;
			camera.Set(Cam::MODE::FIRST_PERSON);

			if (difficulty == 0) {
				DataManager::GetInstance().SaveData(DataManager::RHYTHM_SCORE_DIFF0, RhythmGameManager::GetInstance().GetScore());
				DataManager::GetInstance().UpdateData(DataManager::RHYTHM_SCORE_DIFF0);
			}
			else {
				DataManager::GetInstance().SaveData(DataManager::RHYTHM_SCORE_DIFF1, RhythmGameManager::GetInstance().GetScore());
				DataManager::GetInstance().UpdateData(DataManager::RHYTHM_SCORE_DIFF1);
			}

			float balanceMult;
			if (difficulty == 0)
				balanceMult = 45000;
			else
				balanceMult = 65000;

			AmplifiedCollabScore = collabScore * (RhythmGameManager::GetInstance().GetScore() / balanceMult);

			currentState = RESULT;
		}
		break;

	case RESULT:
		dynamicStateTimer = 0;
		resultSFXTimer += dt;

		if (resultSFXTimer >= 0.75f && resultSFXTimer < 1) {
			resultSFXTimer = 1;

			if (RhythmGameManager::GetInstance().GetProgressionFixed() && RhythmGameManager::GetInstance().GetProgressionMaxed())
				AudioManager::GetInstance().PlaySFX(SFX_TADA);
			else 
				AudioManager::GetInstance().PlaySFX(SFX_FAIL);
		}
		break;

	case END_RESULT:
		resultSFXTimer = 0;
		if (dynamicStateTimer > 3) {
			dynamicStateTimer = 0;

			DialogueManager::GetInstance().EndDialogue();

			currentState = START_INTERMISSION;
		}
		break;

	case EXIT:
		DataManager::GetInstance().SaveData(DataManager::COLLAB_SCORE, collabScore);
		DataManager::GetInstance().UpdateData(DataManager::COLLAB_SCORE);

		SceneManager::GetInstance().RequestChangeState(new SceneMedical());
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

		switch (currentState) {
		case START_GAME:
			obj->allowRender = true;
			obj->alpha = Smooth(obj->alpha, 1.f, 30, dt);
			break;
		case GAME:
			obj->alpha = 1;
			break;
		case START_RESULT:
			obj->alpha = Smooth(obj->alpha, 0.f, 20, dt);
			break;
		default:
			obj->alpha = 0.f;
			break;
		}

		if (obj->geometryType == RHYTHM_HIT_POINT) {
			int index = std::stoi(pair.first.substr(5, 1));
			auto ColorByLane = [](int laneIndex) {
				switch (laneIndex) {
				case 0: return vec3(1.f, 0.5f, 0.5f);
				case 1: return vec3(1.f, 1.f, 0.5f);
				case 2: return vec3(0.5f, 1.f, 1.f);
				case 3: return vec3(0.5f, 1.f, 0.5f);
				default: return vec3(1);
				}
				};

			obj->trl = RhythmGameManager::GetInstance().GetLanes()[index].position;

			if (RhythmGameManager::GetInstance().GetHeldLane(index)) {
				obj->scl = Smooth(obj->scl, vec3(0.9f), 2.5, dt);
				obj->colorFilter = Average(ColorByLane(index), vec3(1));
			}
			else {
				obj->scl = Smooth(obj->scl, vec3(1), 20, dt);
				obj->colorFilter = Smooth(obj->colorFilter, ColorByLane(index), 20, dt);
			}
		}

		if (obj->geometryType == RT_PROGRESSION) {
			static float accel = 0;
			switch (currentState) {
			case START_GAME:
				obj->trl = Smooth(obj->trl, vec3(1.25f, 2.525f, -0.55f), 15, dt);
				obj->rot = Smooth(obj->rot, vec3(40, -60, 40), 15, dt);
				break;
			case START_INTERMISSION:
			case START_RESULT:
				obj->rot = Smooth(obj->rot, vec3(0), 25, dt);
				obj->trl = Smooth(obj->trl, vec3(3, 1.6f, 0.035f), 15, dt);
				break;
			case RESULT:
				obj->alpha = 1.f;
				obj->trl = Smooth(obj->trl, vec3(0, 1.6f, 0.035f), 25, dt);
				break;
			case END_RESULT:
				obj->alpha = 1.f;
				accel += 5 * dt;
				obj->trl.x += accel * dt;
			default: break;
			}
		}

		if (obj->name == "progress bar") {
			const auto& progression = RhythmGameManager::GetInstance().GetProgression();
			
			obj->ClearChildren();

			auto& newObj = RenderObject::newObject;

			obj->NewChild(MeshObject::Create(RT_PROGRESS_INDICATOR));
			newObj->trl = vec3(-0.505f, 0, 0.005f);
			obj->NewChild(MeshObject::Create(RT_PROGRESS_INDICATOR));
			newObj->trl = vec3(0.505f, 0, 0.005f);

			float accumulatedOffsetPosition = 0 - 0.5f;
			for (auto& progress : progression) {
				obj->NewChild(MeshObject::Create(RT_PROGRESS));
				if (progress.type == RhythmGameManager::Progress::SUCCESS)
					newObj->colorFilter = vec3(0.5f, 1, 0.5f);
				else 
					newObj->colorFilter = vec3(1, 0.5f, 0.5f);
				newObj->trl.x = accumulatedOffsetPosition;
				newObj->scl.x = progress.amount / RhythmGameManager::GetInstance().GetMaxProgression();
				accumulatedOffsetPosition += newObj->scl.x;
			}
			newObj.reset();

			switch (currentState) {
			case RESULT:
			case END_RESULT:
				obj->alpha = 1.f;
			default: break;
			}
		}
	}

	// ui poi
	for (auto& pair : uiPointsOfInterest) {
		auto obj = pair.second.lock();

		if (obj->name.find("compressed") != string::npos || obj->name.find("lossless") != string::npos || obj->name.find("upload") != string::npos) {
			switch (currentState) {
			case START_INTERMISSION:
			case INTERMISSION:
			case END_INTERMISSION:
				obj->alpha = 1;
				break;
			case START_RESULT:
			case RESULT:
			case END_RESULT:
				break;
			default: obj->alpha = 0; break;
			}
		}
		else if (obj->name.find("next") != string::npos || obj->name.find("retry") != string::npos) {
			switch (currentState) {
			case RESULT:
				obj->alpha = Smooth(obj->alpha, 1.f, 60, dt);
				break;
			case END_RESULT:
				obj->alpha = 1;
				break;
			default: obj->alpha = 0; break;
			}
		}
		else if (obj->name == "rt base ui") {
			static float accel = 0;
			switch (currentState) {
			case START_INTERMISSION:
				meshList[obj->geometryType]->textureID = baseUITexture[0];
				accel = 0;
				obj->trl = Smooth(obj->trl, vec3(0, 1.5f, 0), 25, dt);
				break;
			case INTERMISSION:
				break;
			case END_INTERMISSION:
				accel += 5 * dt;
				obj->trl.x += accel * dt;
				break;
			case START_RESULT:
				meshList[obj->geometryType]->textureID = baseUITexture[1];
				accel = 0;
				obj->trl = Smooth(obj->trl, vec3(0, 1.5f, 0), 25, dt);
				break;
			case RESULT:
				break;
			case END_RESULT:
				accel += 5 * dt;
				obj->trl.x -= accel * dt;
				break;
			default: break;
			}
		}
		else if (obj->name == "panel ui") {
			static float accel = 0;
			switch (currentState) {
			case START_INTERMISSION:
				accel = 0;
				obj->trl = Smooth(obj->trl, vec3(0, 1.6f, 0.035f), 25, dt);
				obj->rot = Smooth(obj->rot, vec3(0, 0, 0.035f), 25, dt);
				break;
			case INTERMISSION:
				break;
			case END_INTERMISSION:
				accel += 5 * dt;
				obj->trl.x -= accel * dt;
				break;
			case START_GAME:
				obj->trl = Smooth(obj->trl, vec3(-1.25f, 2.525f, -0.55f), 15, dt);
				obj->rot = Smooth(obj->rot, vec3(40, 60, -40), 15, dt);
				break;
			case START_RESULT:
				obj->rot = Smooth(obj->rot, vec3(0, 0, 0.035f), 25, dt);
				accel += 5 * dt;
				obj->trl.x -= accel * dt;
				break;
			case RESULT:
				break;
			case END_RESULT:
				break;
			default: break;
			}
		}
		else if (obj->name == "disc") {
			switch (currentState) {
			case START_INTERMISSION:
				obj->rot.z = 0;
				break;
			case GAME:
				obj->rot.z -= 30 * dt;
				break;
			default: break;
			}
		}
		else if (obj->name == "high score") {
			switch (currentState) {
			case RESULT: {
				if (resultSFXTimer >= 1) {
					obj->alpha = Smooth(obj->alpha, 0.5f, 50, dt);
				}
				
				auto textObj = std::static_pointer_cast<TextObject>(obj);

				if (difficulty == 0)
					textObj->text = "highscore: " + std::to_string(DataManager::GetInstance().GetThisHighScoreData(DataManager::RHYTHM_SCORE_DIFF0));
				else 
					textObj->text = "highscore: " + std::to_string(DataManager::GetInstance().GetThisHighScoreData(DataManager::RHYTHM_SCORE_DIFF1));

				break;
			}
			case END_RESULT:
				obj->alpha = 0.5f;
				break;
			default:
				obj->alpha = 0;
				break;
			}
		}
		else if (obj->name == "new high score") {
			switch (currentState) {
			case RESULT: {
				if (resultSFXTimer >= 1.5) {
					obj->alpha = Smooth(obj->alpha, 0.5f, 20, dt);
				}

				auto textObj = std::static_pointer_cast<TextObject>(obj);

				if (difficulty == 0 && DataManager::GetInstance().LocalHighScoreChanged(DataManager::RHYTHM_SCORE_DIFF0))
					textObj->text = "NEW HIGHSCORE";
				else if (difficulty == 1 && DataManager::GetInstance().LocalHighScoreChanged(DataManager::RHYTHM_SCORE_DIFF1))
					textObj->text = "NEW HIGHSCORE";
				else {
					textObj->text = "";
				}

				break;
			}
			case END_RESULT:
				obj->alpha = 0.5f;
				break;
			default:
				obj->alpha = 0;
				break;
			}
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

		if (auto textObj = std::dynamic_pointer_cast<TextObject>(obj)) {
			if (textObj->name.find("dial_t") != std::string::npos) {

				textObj->colorFilter = situationTextColor;

				if (DialogueManager::GetInstance().CheckActivePack()) {
					textObj->text = DialogueManager::GetInstance().GetVisibleLine();
				}
				else
					textObj->text = "";
			}
			if (textObj->name.find("scores") != std::string::npos) {
				if (RhythmGameManager::GetInstance().GetProgressionMaxed() && RhythmGameManager::GetInstance().GetProgressionFixed() || currentState == START_RESULT || currentState == RESULT) {
					textObj->text = std::to_string(RhythmGameManager::GetInstance().GetScore());
					textObj->text = "score: " + textObj->text.substr(0, textObj->text.find("."));

					textObj->alpha = Smooth(textObj->alpha, 0.5f, 60, dt);
				}

				if (currentState == START_GAME) {
					textObj->alpha = 0;
				}
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
			if (textObj->name == "comment") {
				if (currentState == RESULT) {
					commentTimer += dt;

					if (AmplifiedCollabScore > collabScore) {
						textObj->text = "you are doing great! Next game will be easier for you.";
						textObj->color = vec3(0.01f, 1, 0.337f);
					}
					else {
						textObj->text = "you are not doing so well, next game will be harder for you.";
						textObj->color = vec3(1, 0.01f, 0.337f);
					}

					if (commentTimer > 1.5f && commentTimer <= 2.5f) {
						textObj->alpha = Ease(EASE::IN_OUT_SIN, LerpTime(commentTimer, 1.5f, 2.5f));
					}
					else if (commentTimer > 2.5f) {
						textObj->alpha = 1;
					}
				}
				else if (currentState == END_RESULT) {
					commentTimer == 0;
					textObj->alpha = Smooth(textObj->alpha, 0.f, 25, dt);
				}
				else {
					textObj->alpha = 0;
				}
			}

			if (obj->name == "controls") {
				if (currentState == END_INTERMISSION || currentState == GAME || currentState == START_GAME) {
					obj->alpha = Smooth(obj->alpha, 1.f, 25, dt);
				}
				else if (currentState == START_RESULT) {
					obj->alpha = Smooth(obj->alpha, 0.f, 25, dt);
				}
				else {
					obj->alpha = 0;
				}
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
		properties.power *= obj->accumulatedAlpha;

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
	switch (currentState) {
	case START_INTERMISSION:
	case INTERMISSION:
	case END_INTERMISSION:
	case START_RESULT:
	case RESULT:
	case END_RESULT: {
		physicsRaycast.ClearInfo();
		rp3d::Ray ray = MakeRay(camera.GetFinalPosition(), camera.GetFinalPosition() + camera.GetFinalDirection(), 2);
		PhysicsManager::GetInstance().GetWorld()->raycast(ray, &physicsRaycast);
	}
	default: break;
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
							uploadBg->scl.y = Smooth(uploadBg->scl.y, 1.f, 5, dt);
							uploadBtn->scl = Smooth(uploadBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							uploadBg->scl.y = Smooth(uploadBg->scl.y, 1.f, 10.f, dt);
							uploadBtn->scl = Smooth(uploadBtn->scl, vec3(1), 10.f, dt);
						}

						if (lmb_released && currentState == INTERMISSION) {
							currentState = END_INTERMISSION;
							AudioManager::GetInstance().PlaySFX(SFX_CLICK);
						}
					}
					else {
						uploadBg->scl.y = Smooth(uploadBg->scl.y, 0.f, 10.f, dt);
						uploadBtn->scl = Smooth(uploadBtn->scl, vec3(1), 10.f, dt);
					}
				}
				else if (obj->name == "lossless trigger") {
					auto losslessBg = uiPointsOfInterest["lossless bg"].lock();
					physics->SetPosition(getPosFromModel(losslessBg->model));
					auto losslessBtn = uiPointsOfInterest["lossless btn"].lock();
					
					float mult = 1;
					if (difficulty == 1) {
						mult = 1.5f;
					}

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							losslessBg->scl.y = Smooth(losslessBg->scl.y, 1.f, 10.f, dt);
							losslessBtn->scl = Smooth(losslessBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							losslessBg->scl.y = Smooth(losslessBg->scl.y, 1.f, 10.f, dt);
							losslessBtn->scl = Smooth(losslessBtn->scl, vec3(1) * mult, 10.f, dt);
						}

						if (lmb_released && currentState == INTERMISSION) {
							difficulty = 1;
							AudioManager::GetInstance().PlaySFX(SFX_CLICK);
						}
					}
					else {
						losslessBg->scl.y = Smooth(losslessBg->scl.y, 0.f, 10.f, dt);
						losslessBtn->scl = Smooth(losslessBtn->scl, vec3(1) * mult, 10.f, dt);
					}
				}
				else if (obj->name == "compressed trigger") {
					auto compressedBg = uiPointsOfInterest["compressed bg"].lock();
					physics->SetPosition(getPosFromModel(compressedBg->model));
					auto compressedBtn = uiPointsOfInterest["compressed btn"].lock();

					float mult = 1;
					if (difficulty == 0) {
						mult = 1.5f;
					}

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							compressedBg->scl = Smooth(compressedBg->scl, vec3(1), 10.f, dt);
							compressedBtn->scl = Smooth(compressedBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							compressedBg->scl = Smooth(compressedBg->scl, vec3(1), 10.f, dt);
							compressedBtn->scl = Smooth(compressedBtn->scl, vec3(1) * mult, 10.f, dt);
						}

						if (lmb_released && currentState == INTERMISSION) {
							difficulty = 0;
							AudioManager::GetInstance().PlaySFX(SFX_CLICK);
						}
					}
					else {
						compressedBg->scl = Smooth(compressedBg->scl, vec3(1, 0, 1), 10.f, dt);
						compressedBtn->scl = Smooth(compressedBtn->scl, vec3(1) * mult, 10.f, dt);
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
				if (obj->name == "next trigger") {
					auto nextBg = uiPointsOfInterest["next bg"].lock();
					physics->SetPosition(getPosFromModel(nextBg->model));
					auto nextBtn = uiPointsOfInterest["next btn"].lock();

					float mult = 1;

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							nextBg->scl = Smooth(nextBg->scl, vec3(1), 10.f, dt);
							nextBtn->scl = Smooth(nextBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							nextBg->scl = Smooth(nextBg->scl, vec3(1), 10.f, dt);
							nextBtn->scl = Smooth(nextBtn->scl, vec3(1) * mult, 10.f, dt);
						}

						if (lmb_released && currentState == RESULT) {
							currentState = EXIT;
							AudioManager::GetInstance().PlaySFX(SFX_CLICK);
						}
					}
					else {
						nextBg->scl = Smooth(nextBg->scl, vec3(1, 0, 1), 10.f, dt);
						nextBtn->scl = Smooth(nextBtn->scl, vec3(1) * mult, 10.f, dt);
					}
				}
				else if (obj->name == "retry trigger") {
					auto retryBg = uiPointsOfInterest["retry bg"].lock();
					physics->SetPosition(getPosFromModel(retryBg->model));
					auto retryBtn = uiPointsOfInterest["retry btn"].lock();

					float mult = 1;

					auto index = physicsRaycast.FindHit(physics->Getbody());
					if (index != -1) {
						const auto& raycastInfo = physicsRaycast.GetRaycastInfos()[index];

						if (lmb_down) {
							retryBg->scl = Smooth(retryBg->scl, vec3(1), 10.f, dt);
							retryBtn->scl = Smooth(retryBtn->scl, vec3(0.9f), 5, dt);
						}
						else {
							retryBg->scl = Smooth(retryBg->scl, vec3(1), 10.f, dt);
							retryBtn->scl = Smooth(retryBtn->scl, vec3(1) * mult, 10.f, dt);
						}

						if (lmb_released && currentState == RESULT) {
							currentState = END_RESULT;
							AudioManager::GetInstance().PlaySFX(SFX_CLICK);
						}
					}
					else {
						retryBg->scl = Smooth(retryBg->scl, vec3(1, 0, 1), 10.f, dt);
						retryBtn->scl = Smooth(retryBtn->scl, vec3(1) * mult, 10.f, dt);
					}
				}
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
	camera.Update(dt);

	AddDebugText("camera.finalPosition: " + VecToString(camera.GetPlainPosition()));
	AddDebugText("RGM.currentBeat: " + std::to_string(RhythmGameManager::GetInstance().GetCurrentBeat()));
	string progression_s = "";
	for (auto& progress : RhythmGameManager::GetInstance().GetProgression()) {
		if (progress.type == RhythmGameManager::Progress::SUCCESS) {
			progression_s += "S:" + std::to_string(progress.amount) + " ";
		}
		else {
			progression_s += "L:" + std::to_string(progress.amount) + " ";
		}
	}
	AddDebugText("RGM.progress: " + progression_s);
	AddDebugText("AudioManager::MUSVolume: " + std::to_string(AudioManager::GetInstance().VolumeMUS(-1)));

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
		RhythmGameManager::GetInstance().SetAutoPlay(debug);
	}
	forceEndGame = false;
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

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E)) {
			forceEndGame = true;
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
	/*if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE)) {

		DialogueManager::GetInstance().StartDialogue("file_path");

		if (DialogueManager::GetInstance().CheckActivePack()) {
			DialogueManager::GetInstance().ControlCurrentDialogue();
		}
	}*/

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

	for (int i = 0; i < RhythmGameManager::GetInstance().GetLanes().size(); i++) {
		RhythmGameManager::GetInstance().SetTappedLane(i, false);
		RhythmGameManager::GetInstance().SetHeldLane(i, false);
	}
	if (currentState == GAME) {
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_F))
			RhythmGameManager::GetInstance().SetTappedLane(0, true);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_F))
			RhythmGameManager::GetInstance().SetHeldLane(0, true);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_G))
			RhythmGameManager::GetInstance().SetTappedLane(1, true);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_G))
			RhythmGameManager::GetInstance().SetHeldLane(1, true);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_H))
			RhythmGameManager::GetInstance().SetTappedLane(2, true);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_H))
			RhythmGameManager::GetInstance().SetHeldLane(2, true);

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_J))
			RhythmGameManager::GetInstance().SetTappedLane(3, true);
		if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_J))
			RhythmGameManager::GetInstance().SetHeldLane(3, true);
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
