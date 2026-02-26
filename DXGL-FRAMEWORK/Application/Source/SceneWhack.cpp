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
			PhysicsManager::GetInstance().SetUpLogger("SceneWhack");
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
		AudioManager::GetInstance().LoadMUS("mental_pollution.ogg", 154.0000);

		// sfx init
		AudioManager::GetInstance().LoadSFX(HIT, "boom_swoosh.wav");
		AudioManager::GetInstance().VolumeSFX(HIT, 1.f);

		AudioManager::GetInstance().LoadSFX(VIRUS_DIE, "pixel_explode.wav");
		AudioManager::GetInstance().VolumeSFX(VIRUS_DIE, 0.4f);

		AudioManager::GetInstance().LoadSFX(NEWRECORD, "happy_wheels_win.wav");
		AudioManager::GetInstance().VolumeSFX(HIT, 1.f);

		AudioManager::GetInstance().LoadSFX(OKAYSCORE, "sparkle.wav");
		AudioManager::GetInstance().VolumeSFX(HIT, 1.f);

		AudioManager::GetInstance().LoadSFX(ZEROSCORE, "sadtrombone.wav");
		AudioManager::GetInstance().VolumeSFX(HIT, 1.f);
	}

	// dialogue init
	{
		DialogueManager::GetInstance().LoadDialoguePack("CarnivalIntro.json");
		DialogueManager::GetInstance().LoadDialoguePack("Tutorial1.json");
		DialogueManager::GetInstance().LoadDialoguePack("Tutorial2.json");
		DialogueManager::GetInstance().LoadDialoguePack("Tutorial3.json");
		DialogueManager::GetInstance().LoadDialoguePack("Tutorial4.json");
		DialogueManager::GetInstance().LoadDialoguePack("Tutorial5.json");
		DialogueManager::GetInstance().LoadDialoguePack("GameIntro.json");
		DialogueManager::GetInstance().LoadDialoguePack("EndGame.json");
		DialogueManager::GetInstance().LoadDialoguePack("EndGame_NewRecord.json");
		DialogueManager::GetInstance().LoadDialoguePack("EndGame_ZeroScore.json");
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
		meshList[LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));
		meshList[DEBUG_LINE] = MeshBuilder::GenerateLine("debug line", 1);
		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, FontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTexture("Cascadia_Mono.tga"));
		meshList[SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTexture("skybox_black.tga"));
		
		meshList[GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTexture("circuit.tga"));
		meshList[PLATFORM] = MeshBuilder::GenerateGround("platform", 20, 5, TextureLoader::LoadTexture("circuit.tga"));
		meshList[SCI_FLOOR] = MeshBuilder::GenerateOBJ("sci floor", "sci_floor.obj", TextureLoader::LoadTexture("sci_floor.tga"));
		meshList[SCI_WALL] = MeshBuilder::GenerateQuad("sci wall", vec3(1), 1, 1, TextureLoader::LoadTexture("sci_wall.tga"));
		meshList[PHYSICS_BOX] = MeshBuilder::GenerateCube("physics box", vec3(1), 1);
		meshList[TERMINAL] = MeshBuilder::GenerateOBJMTL("terminal", "terminal.obj", "terminal.mtl", TextureLoader::LoadTexture("terminal.png"));
		meshList[CRATE] = MeshBuilder::GenerateOBJMTL("crate", "crate.obj", "crate.mtl", TextureLoader::LoadTexture("crate.png"));
		meshList[TELEPORTER] = MeshBuilder::GenerateOBJMTL("teleporter", "teleporter.obj", "teleporter.mtl", TextureLoader::LoadTexture("teleporter.png"));

		meshList[ANTIVIRUS_BALL] = MeshBuilder::GenerateSphere("antivirus ball", vec3(1.f), 0.5f, 16, 8, TextureLoader::LoadTexture("antivirus.tga"));
		meshList[TRIGGER_BOX] = MeshBuilder::GenerateCube("trigger box", vec3(1.f), 2);

		meshList[VIRUS_1] = MeshBuilder::GenerateOBJ("virus1", "virus_one.obj");

		meshList[PORTAL_EVIL] = MeshBuilder::GenerateQuad("portal evil", vec3(1), 1, 1, TextureLoader::LoadTexture("portal.tga"));
		meshList[PORTAL_GOOD] = MeshBuilder::GenerateQuad("portal good", vec3(1), 1, 1, TextureLoader::LoadTexture("portal.tga"));

		meshList[CROSSHAIR] = MeshBuilder::GenerateQuad("crosshair", vec3(1), 1, 1, TextureLoader::LoadTexture("crosshair.tga"));

		meshList[WHITE] = MeshBuilder::GenerateQuad("white", vec3(1), 1, 1, TextureLoader::LoadTexture("white.png"));
		meshList[GAUGE] = MeshBuilder::GenerateQuad("gauge", vec3(1), 1, 1, TextureLoader::LoadTexture("gauge.png"));
		meshList[TUTORIAL] = MeshBuilder::GenerateQuad("tutorial", vec3(1), 1, 1, TextureLoader::LoadTexture("tutorial.tga"));
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
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 |CATEGORY_3 | CATEGORY_4);
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->UpdateMassProperties();

			});
		RObj::setDefaultStat.Subscribe(SCI_FLOOR, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::POLISHED_METAL);
			});
		RObj::setDefaultStat.Subscribe(SCI_WALL, [](const std::shared_ptr<RObj>& obj) {
			obj->material.Set(Material::POLISHED_METAL);
			obj->colorFilter = vec3(0.4f, 0.4f, 0.6f);
			});
		RObj::setDefaultStat.Subscribe(PHYSICS_BOX, [](const std::shared_ptr<RObj>& obj) {
			obj->offsetScl = vec3(2.f, 20.f, 20.f);
			obj->AddPhysics(PhysicsObject::STATIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(1.f, 10.f, 10.f));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 | CATEGORY_3);
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
		RObj::setDefaultStat.Subscribe(CRATE, [](const std::shared_ptr<RObj>& obj) {
			obj->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::BOX, vec3(0.9f, 1.f, 0.9f), vec3(0.f, 1.f, 0.f));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 | CATEGORY_3);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(CRATE, [&](const std::shared_ptr<RObj>& obj) { // these specific lines is required for all physics object that can be deleted in the programe, its safe to add it for all physics objects to be safe in case if you missed any
			dirtyWorldList = true;
			});
		RObj::setDefaultStat.Subscribe(TELEPORTER, [](const std::shared_ptr<RObj>& obj) {
			obj->name = "teleporter";
			obj->colorFilter = vec3(0.6f, 0.8f, 1.f);
			obj->offsetScl = vec3(0.1f, 0.1f, 0.1f);
			obj->AddPhysics(PhysicsObject::STATIC);
			auto physics = obj->GetPhysics();
			physics->AddCollider(PhysicsObject::SPHERE, vec3(0.4f), vec3(0.f, 0.4f, 0.f));
			physics->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
			physics->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 | CATEGORY_3);
			physics->SetTrigger(true);
			physics->UpdateMassProperties();
			});
		RObj::setDestroyedEvent.Subscribe(TELEPORTER, [&](const std::shared_ptr<RObj>& obj) { // these specific lines is required for all physics object that can be deleted in the programe, its safe to add it for all physics objects to be safe in case if you missed any
			dirtyWorldList = true;
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
			physics->AddCollider(PhysicsObject::BOX, vec3(2.f, 1.f, 2.f));
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
			obj->colorFilter = vec3(0, 1, 0);
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
			obj->material.Set(Material::NO_LIGHT);
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

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				worldRoot->NewChild(MeshObject::Create(SCI_FLOOR));
				newObj->trl = vec3(-9.f + (i * 2.f), 6.f, -9.f + (j * 2.f));
			}
		}
		
		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < 5; i++)
			{
				worldRoot->NewChild(MeshObject::Create(SCI_WALL));
				newObj->trl = vec3(-8.f + (i * 4.f), 8.f + (j * 4.f), -10.f);
				newObj->scl = vec3(4.f);
			}

			for (int i = 0; i < 5; i++)
			{
				worldRoot->NewChild(MeshObject::Create(SCI_WALL));
				newObj->trl = vec3(-8.f + (i * 4.f), 8.f + (j * 4.f), 10.f);
				newObj->rot.y = 180.f;
				newObj->scl = vec3(4.f);
			}

			for (int i = 0; i < 5; i++)
			{
				worldRoot->NewChild(MeshObject::Create(SCI_WALL));
				newObj->trl = vec3(-10.f, 8.f + (j * 4.f), -8.f + (i * 4.f));
				newObj->rot.y = 90.f;
				newObj->scl = vec3(4.f);
			}
		}

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				worldRoot->NewChild(MeshObject::Create(SCI_WALL));
				newObj->trl = vec3(-8.f + (i * 4.f), 18.f, -8.f + (j * 4.f));
				newObj->rot.x = 90.f;
				newObj->scl = vec3(4.f);
			}
		}

		worldRoot->NewChild(MeshObject::Create(CRATE));
		newObj->GetPhysics()->SetPosition(vec3(-5.f, 6.5f, 0.f));

		worldRoot->NewChild(MeshObject::Create(CRATE));
		newObj->GetPhysics()->SetPosition(vec3(2.f, 6.5f, -5.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 45.f, 0.f));

		worldRoot->NewChild(MeshObject::Create(CRATE));
		newObj->GetPhysics()->SetPosition(vec3(-2.f, 6.5f, -5.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 15.f, 0.f));

		worldRoot->NewChild(MeshObject::Create(CRATE));
		newObj->offsetScl = vec3(0.5f);
		newObj->RemovePhysics();
		newObj->AddPhysics(PhysicsObject::STATIC);
		newObj->GetPhysics()->AddCollider(PhysicsObject::BOX, vec3(0.45f, 0.5f, 0.45f), vec3(0.f, 0.5f, 0.f));
		newObj->GetPhysics()->Getbody()->getCollider(0)->setCollisionCategoryBits(CATEGORY_2);
		newObj->GetPhysics()->Getbody()->getCollider(0)->setCollideWithMaskBits(CATEGORY_1 | CATEGORY_2 | CATEGORY_3);
		scoreDisplayPosition = vec3(4.f, 6.f, 5.f);
		newObj->GetPhysics()->SetPosition(scoreDisplayPosition);
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 30.f, 0.f));
		newObj->GetPhysics()->UpdateMassProperties();

		worldRoot->NewChild(MeshObject::Create(GROUP));
		newObj->name = "score_display";
		newObj->trl = vec3(2.f, 6.5f, 5.f);
		auto parentObj = newObj; 
		
		parentObj->NewChild(MeshObject::Create(WHITE));
		newObj->alpha = 0.5f;
		newObj->hasTransparency = true;
		newObj->colorFilter = vec3(0.5f, 0.75f, 1.f);
		newObj->name = "score_panel";
		newObj->trl = vec3(0.f, 0.f, 0.f);
		newObj->offsetTrl = vec3(0.f, 0.5f, 0.f);
		newObj->scl = vec3(0.8f, 1.5f, 0.8f);

		parentObj->NewChild(TextObject::Create("score_text", "High score:", vec3(0), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(0.f, 1.f, 0.f);
		newObj->scl = vec3(0.15f);

		parentObj->NewChild(TextObject::Create("score_record", "High score:", vec3(0), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(0.f, 0.8f, 0.f);
		newObj->scl = vec3(0.15f);

		worldRoot->NewChild(MeshObject::Create(TELEPORTER));
		newObj->GetPhysics()->SetPosition(vec3(7.f, 6.f, -7.f));

		worldRoot->NewChild(TextObject::Create("teleporter_text", "Go to next scene", vec3(0), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(7.f, 7.f, -7.f);
		newObj->scl = vec3(0.15f);

		worldRoot->NewChild(MeshObject::Create(PHYSICS_BOX));
		newObj->name = "glass";
		newObj->colorFilter = vec3(0.5f, 0.75f, 1.f);
		newObj->alpha = 0.5f;
		newObj->hasTransparency = true;
		newObj->GetPhysics()->SetPosition(vec3(10.f, 8.f, 0.f));
		newObj->GetPhysics()->SetAllowSleep(false);

		worldRoot->NewChild(MeshObject::Create(PHYSICS_BOX));
		newObj->alpha = 0.f;
		newObj->hasTransparency = true;
		newObj->GetPhysics()->SetPosition(vec3(-10.f, 8.f, 0.f));
		newObj->GetPhysics()->SetAllowSleep(false);

		worldRoot->NewChild(MeshObject::Create(PHYSICS_BOX));
		newObj->alpha = 0.f;
		newObj->hasTransparency = true;
		newObj->GetPhysics()->SetPosition(vec3(0.f, 8.f, 10.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 90.f, 0.f));
		newObj->GetPhysics()->SetAllowSleep(false);

		worldRoot->NewChild(MeshObject::Create(PHYSICS_BOX));
		newObj->alpha = 0.f;
		newObj->hasTransparency = true;
		newObj->GetPhysics()->SetPosition(vec3(0.f, 8.f, -10.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, 90.f, 0.f));
		newObj->GetPhysics()->SetAllowSleep(false);

		
		worldRoot->NewChild(MeshObject::Create(TERMINAL));
		newObj->name = "terminal";
		newObj->GetPhysics()->SetPosition(vec3(8.f, 6.f, 0.f));
		newObj->GetPhysics()->SetOrientation(vec3(0.f, -90.f, 0.f));

		worldRoot->NewChild(TextObject::Create("terminal_text", "Play game here!", vec3(0), FONT_CASCADIA_MONO, true));
		newObj->trl = vec3(8.f, 8.f, 0.f);
		newObj->scl = vec3(0.15f);

		InitPortalMaps();

		worldRoot->NewChild(MeshObject::Create(CROSSHAIR));
		newObj->name = "crosshair";
		newObj->scl = vec3(3);
		newObj->trl = vec3(0, 1.f, 0);
		newObj->rot.y = -90.f;
		newObj->allowRender = false;

		worldRoot->NewChild(MeshObject::Create(PORTAL_GOOD));
		newObj->name = "portal_ground";
		newObj->alpha = 0.f;
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
				newLightObj->trl = vec3(0.f, 6.5f, 0.f);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_POINT;
				lightProperties.color = vec3(1.f);
				lightProperties.power = 0.5f;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.001f;
				lightProperties.kQ = 0.001f;
				UpdateLightUniform(newLightObj);
			}

			worldRoot->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(7.f, 10.f, -7.f);
				newLightObj->name = "teleporterLight";
				newLightObj->initialDire = vec3(0, -1, 0); // must have this to define the initial spotDirection for spot light, default vec3(0, -1, 0)
				newLightObj->rot = vec3(0, 0, 0);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_SPOT;
				lightProperties.color = vec3(1.f, 1.f, 1.f);
				lightProperties.power = 4;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.005f;
				lightProperties.kQ = 0.01f;
				// spot light variables (yes, these are the only 2 you need to change manually)
				lightProperties.cosCutoff = 30.f;
				lightProperties.cosInner = 25.f;
				UpdateLightUniform(newLightObj);
			}

			worldRoot->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(0.f, 10.f, 0.f);
				newLightObj->name = "roomLight";
				newLightObj->initialDire = vec3(0, -1, 0); // must have this to define the initial spotDirection for spot light, default vec3(0, -1, 0)
				newLightObj->rot = vec3(0, 0, 0);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_SPOT;
				lightProperties.color = vec3(1.f, 1.f, 1.f);
				lightProperties.power = 5;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.005f;
				lightProperties.kQ = 0.01f;
				// spot light variables (yes, these are the only 2 you need to change manually)
				lightProperties.cosCutoff = 50.f;
				lightProperties.cosInner = 30.f;
				UpdateLightUniform(newLightObj);
			}

			worldRoot->NewChild(LightObject::Create(LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(newObj);
			{
				newLightObj->trl = vec3(40.f, 20.f, 0.f);
				newLightObj->name = "gameLight";
				newLightObj->initialDire = vec3(0, -1, 0); // must have this to define the initial spotDirection for spot light, default vec3(0, -1, 0)
				newLightObj->rot = vec3(0, 0, 0);
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_SPOT;
				lightProperties.color = vec3(1.f, 1.f, 1.f);
				lightProperties.power = 0.001f;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.005f;
				lightProperties.kQ = 0.01f;
				// spot light variables (yes, these are the only 2 you need to change manually)
				lightProperties.cosCutoff = 200.f;
				lightProperties.cosInner = 150.f;
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

		screenRoot->NewChild(MeshObject::Create(TUTORIAL, 1)); // create with 1 as UILayer, default 0
		newObj->relativeTrl = true;
		newObj->name = "tutorial";
		newObj->trl = vec3(0.f, 0.25f, 0);
		newObj->scl = vec3(600, 600, 1);

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

		screenRoot->NewChild(TextObject::Create("press_start", "Press Z to start game!", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0.f, -0.8f, 0);
		newObj->scl = vec3(80, 80, 1);

		screenRoot->NewChild(TextObject::Create("new_record", "NEW RECORD!", vec3(1), FONT_CASCADIA_MONO, true));
		newObj->relativeTrl = true;
		newObj->trl = vec3(0.f, 0.4f, 0);
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

	debugPhysicsTimer += dt;
	elapsed += dt;
	startGameTimer -= dt;

	if (startGameTimer <= 0)
	{
		canStartGame = false;
	}

	auto& lightList = LightObject::lightList;
	auto& worldList = RObj::worldList;
	auto& viewList = RObj::viewList;
	auto& screenList = RObj::screenList;
	auto& physicsList = RObj::physicsList;

	// if you ever felt that you need dt inside HandleKeyPress(), that means you are doing smt wro- i mean, you need to use a variable to pass the info and commit changes in Update instead, HandleKeyPress() should not have those kinda logic inside it
	HandleKeyPress();
	StartCutscene();

	// Game updates
	if (inGame && !endGame)
	{
		gameTimer -= dt;

		if (gameTimer <= 0.f)
		{
			allGameObjectsDestroyed = true;
			isGaugeActive = false;
			if (gameScore > highestScore)
			{
				isNewRecord = true;
			}
		}
		else if ((int)gameTimer % 15 == 0 && gameTimer != 60.f && gameTimer > 1.f && !triggeredChange && !triggeredOnce)
		{
			triggeredChange = true;
			triggeredOnce = true;
		}
		else if ((int)gameTimer % 15 != 0)
		{
			triggeredOnce = false;
		}

		if (triggeredChange)
		{
			MidgameChange();
		}

		auto& newObj = RObj::newObject;
		elapsedVirusInterval -= dt;
		if (elapsedVirusInterval <= 0 && !portalEvilPositionsList.empty())
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
			newVirus.homeX = newObj->GetPhysics()->GetPosition().x;
			newVirus.waypoints = newVirus.BuildRandomPath(portalEvilPositionsList, solsrng, portalDistribution, spawnIndex, true);
			newVirus.waypointIndex = 0;
			virusList.push_back(newVirus);
			elapsedVirusInterval = 5;
		}
	}

	RObj::newObject.reset();

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
			break;

		default:
			break;
		}

		if (obj->name == "crosshair")
		{
			if (!inGame || allGameObjectsDestroyed)
			{
				obj->allowRender = false;
			}
			else if (inGame)
			{
				obj->allowRender = true;
			}
			
			obj->trl = selectedPortalPosition - vec3(5.f, 0.f, 0.f);
			obj->rot.z += 45.f * dt;
		}

		if (obj->name == "score_display")
		{
			vec3 toTarget = obj->trl - player.position;
			toTarget.y = 0.f;
			float distanceSquared = glm::dot(toTarget, toTarget);
			if (distanceSquared < 2.5f * 2.5f || showScoreDisplay)
			{
				obj->alpha = Smooth(obj->alpha, 1.f, 5.f, (float)dt);
			}
			else
			{
				obj->alpha = Smooth(obj->alpha, 0.f, 5.f, (float)dt);
			}
			toTarget = glm::normalize(toTarget);
			float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
			obj->rot.y = desiredYawDeg + 180.f;
		}

		if (auto textObj = std::dynamic_pointer_cast<TextObject>(obj))
		{
			if (textObj->name.find("score_record") != std::string::npos)
			{
				textObj->text = std::to_string(highestScore);
			}

			if (textObj->name.find("teleporter_text") != std::string::npos)
			{
				textObj->color = vec3(1);
				vec3 toTarget = obj->trl - player.position;
				toTarget.y = 0.f;
				toTarget = glm::normalize(toTarget);
				float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
				obj->rot.y = desiredYawDeg + 180.f;
			}

			if (textObj->name.find("terminal_text") != std::string::npos)
			{
				textObj->color = vec3(1);
				vec3 toTarget = obj->trl - player.position;
				toTarget.y = 0.f;
				toTarget = glm::normalize(toTarget);
				float desiredYawDeg = glm::degrees(atan2f(toTarget.x, toTarget.z));
				obj->rot.y = desiredYawDeg + 180.f;
			}
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
			if (isGaugeActive && inGame) obj->allowRender = true;
		}

		if (obj->name == "marker")
		{
			if (isGaugeActive && inGame)
			{
				obj->allowRender = true;
				if (!pauseMarker) gaugeMarkerPosition.x = obj->trl.x = -0.225f * cosf(4.f * (float)elapsed);
				//gaugeMarkerPosition.x = obj->trl.x = -0.225f * cosf(4.f * (float)elapsed);
			}
			else
			{
				gaugeMarkerPosition.x  = obj->trl.x = -0.225f;
				obj->allowRender = false;
			}
		}

		if (obj->name == "tutorial")
		{
			if (showTutorial)
			{
				obj->allowRender = true;
			}
			else
			{
				obj->allowRender = false;
			}
		}

		if (auto textObj = std::dynamic_pointer_cast<TextObject>(obj)) {
			if (textObj->name.find("dial_s") != std::string::npos) {
				if (DialogueManager::GetInstance().CheckActivePack()) {
					if (!hasFinishedCarnivalIntro)
					{
						textObj->color = vec3(1, 1, 0);
						textObj->trl = vec3(0, 0.f, 0);
						textObj->scl = vec3(30, 30, 1);
					}
					else if (gameIntroActive)
					{
						textObj->color = vec3(0.f, 1.f, 0.f);
						textObj->trl = vec3(0, -0.5f, 0);
						textObj->scl = vec3(30, 30, 1);
					}
					else
					{
						textObj->color = vec3(1.f, 1.f, 1.f);
						textObj->trl = vec3(0, -0.5f, 0);
						textObj->scl = vec3(30, 30, 1);
					}
					textObj->text = DialogueManager::GetInstance().GetCurrentSpeaker();
				}
				else
					textObj->text = "";
			}
			if (textObj->name.find("dial_t") != std::string::npos) {
				if (DialogueManager::GetInstance().CheckActivePack()) {
					if (!hasFinishedCarnivalIntro)
					{
						textObj->trl = vec3(0, 0.1f, 0);
						textObj->scl = vec3(60, 60, 1);
					}
					else if (gameIntroActive)
					{
						textObj->color = vec3(0.f, 1.f, 0.f);
						textObj->trl = vec3(0, 0, 0);
						textObj->scl = vec3(100, 100, 1);
					}
					else
					{
						textObj->color = vec3(1.f, 1.f, 1.f);
						textObj->trl = vec3(0, -0.575f, 0);
						textObj->scl = vec3(30, 30, 1);
					}
					textObj->text = DialogueManager::GetInstance().GetVisibleLine();
				}
				else
					textObj->text = "";
			}
			if (textObj->name.find("press_start") != std::string::npos) {
				if (canStartGame)
				{
					textObj->alpha = Smooth(textObj->alpha, 1.f, 6.f, (float)dt);
				}
				else
				{
					textObj->alpha = Smooth(textObj->alpha, 0.f, 6.f, (float)dt);
				}
			}
			if (textObj->name.find("score") != std::string::npos)
			{
				if (inGame)
				{
					textObj->allowRender = true;
					if (endGame)
					{
						textObj->trl = vec3(0.f, 0.6f, 0.f);
						textObj->scl = vec3(120.f, 120.f, 0.f);
					}
					else
					{
						textObj->trl = vec3(0.7f, 0.8f, 0);
						textObj->scl = vec3(80, 80, 1);
					}

					textObj->text = "Score: " + std::to_string(gameScore);
				}
				else
				{
					textObj->allowRender = false;
				}
			}
			if (textObj->name.find("new_record") != std::string::npos)
			{
				if (endGame && isNewRecord)
				{
					textObj->allowRender = true;
				}
				else
				{
					textObj->allowRender = false;
				}

				textObj->color = vec3(0.f, 1.f, 1.f);
			}
			if (textObj->name.find("gameTimer") != std::string::npos)
			{
				if (inGame && !endGame)
				{
					textObj->allowRender = true;
					if (gameTimer < 11.f)
					{
						textObj->color = vec3(1.f, 0.f, 0.f);
					}
					else
					{
						textObj->color = vec3(1.f);
					}
				}
				else
				{
					textObj->allowRender = false;
				}

				textObj->text = "Time left: " + std::to_string(static_cast<int>(gameTimer));
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

		if (obj->name == "gameLight")
		{
			if (isUpdateGameLight)
			{
				if (gameIntroActive)
				{
					properties.power = 5.f;
				}
				else if (endGame)
				{
					properties.power = 0.001f;
				}

				isUpdateGameLight = false;
				UpdateLightUniform(obj);
			}
		}

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

			if (obj->name == "teleporter")
			{
				physics->triggerEvent.Subscribe([&](const rp3d::Body* overlapped) {

					if (player.renderGroup.lock()->GetPhysics()->Getbody() == overlapped)
					{
						if (finishedGameOnce)
						{
							// Insert scene switch code
							// Uses int highestScore from scene data member
							// Score range for grades (if still doing, if not: best score for player is 10)
							// A: >= 10
							// B: 7-9
							// C: 4-6
							// D: 1-3
							// F: 0
						}
					}

					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->triggerEvent, OVERLAP_EVENT::OverlapStart)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
				physics->triggerEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
			}

			if (obj->name == "gamestart_box") {
				physics->triggerEvent.Subscribe([&](const rp3d::Body* overlapped) {
					
					if (player.renderGroup.lock()->GetPhysics()->Getbody() == overlapped)
					{
						canStartGame = true;
						startGameTimer = 0.1;
					}
					
					});
				eventListener.AddToTriggerEvents(PEvent(physics, physics->triggerEvent, OVERLAP_EVENT::OverlapStay)); // add to this so the event gets used for detection, must write correct CONTACT_EVENT or OVERLAP_EVENT
				physics->triggerEvent.lock = true; // lock so it dont subscribe or get added to the triggerEvents again
			}

			if (obj->name == "portal_ground")
			{
				if (!inGame || allGameObjectsDestroyed)
				{
					obj->alpha = Smooth(obj->alpha, 0.f, 4.f, (float)dt);
					if (obj->alpha >= 0.999f) obj->alpha = 1.f;
				}
				else if (inGame)
				{
					obj->alpha = Smooth(obj->alpha, 1.f, 10.f, (float)dt);
					if (obj->alpha <= 0.001f) obj->alpha = 0.f;
				}
				
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
				if (gameChange)
				{
					obj->alpha = Smooth(obj->alpha, 0.f, 10.f, (float)dt);
					if (obj->alpha <= 0.001f) obj->alpha = 0.f;
				}
				else
				{
					obj->alpha = Smooth(obj->alpha, 1.f, 10.f, (float)dt);
					if (obj->alpha >= 0.999f) obj->alpha = 1.f;
				}
				
				int portalIndex = FindEvilPortalIndex(obj);
				if (portalIndex != -1)
				{
					if (allGameObjectsDestroyed)
					{
						portalEvilList.erase(portalEvilList.begin() + portalIndex);
						physicsList.erase(physicsList.begin() + i);
						obj->Destroy();
						obj.reset();
						continue;
					}
					
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
				
				if (gameChange)
				{
					obj->alpha = Smooth(obj->alpha, 0.f, 10.f, (float)dt);
					if (obj->alpha <= 0.001f) obj->alpha = 0.f;
				}
				else
				{
					obj->alpha = Smooth(obj->alpha, 1.f, 10.f, (float)dt);
					if (obj->alpha >= 0.999f) obj->alpha = 1.f;
				}

				int portalIndex = FindGoodPortalIndex(obj);
				if (portalIndex != -1)
				{
					if (allGameObjectsDestroyed)
					{
						portalGoodList.erase(portalGoodList.begin() + portalIndex);
						physicsList.erase(physicsList.begin() + i);
						obj->Destroy();
						obj.reset();
						continue;
					}
					
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

					if (ballTimeList[timerIndex].timer <= 0.f || allGameObjectsDestroyed)
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
								AudioManager::GetInstance().PlaySFX(HIT);
								virusList[j].hasBeenHit = true;
								virusList[j].returningToX = false;
								virusList[j].hp -= 5;
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
					if (physics->GetPosition().x >= 45.f || virusList[virusIndex].hp <= 0)
					{
						virusList[virusIndex].hasBeenDestroyed = true;
					}
					
					if (virusList[virusIndex].hasBeenDestroyed || allGameObjectsDestroyed)
					{
						if (virusList[virusIndex].hasBeenDestroyed)
						{
							AudioManager::GetInstance().PlaySFX(VIRUS_DIE);
							gameScore++;
						}
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
							if (currentVirus.returningToX)
							{
								vec3 virusVelocity = physics->GetVelocity();
								virusVelocity.x = 0.f;
								physics->SetVelocity(virusVelocity);
								
								vec3 virusPosition = physics->GetPosition();
								float differenceX = currentVirus.homeX - virusPosition.x;
								float step = currentVirus.moveSpeed * (float)dt;

								if (fabsf(differenceX) <= 0.05f)
								{
									virusPosition.x = currentVirus.homeX;
									physics->SetPosition(virusPosition);
									currentVirus.returningToX = false;
								}
								else
								{
									virusPosition.x += (differenceX > 0.f ? step : -step);
									physics->SetPosition(virusPosition);
								}
							}
							else
							{
								physics->SetOrientation(vec3(0.f, 90.f, 0.f));
								vec3 virusPosition = physics->GetPosition();
								vec3 targetPosition = currentVirus.waypoints[currentVirus.waypointIndex];

								vec3 virusToTarget = targetPosition - virusPosition;
								virusToTarget.x = 0.f;

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
									virusToTarget.x = 0.f;
									distanceSquared = glm::dot(virusToTarget, virusToTarget);
								}

								if (distanceSquared > 0.000001f)
								{
									vec3 direction = virusToTarget / sqrtf(distanceSquared);
									vec3 desiredVelocity = direction * currentVirus.moveSpeed;
									vec3 currentVelocity = physics->GetVelocity();
									vec3 newVelocity = Smooth(currentVelocity, desiredVelocity, currentVirus.smoothing, (float)dt);
									newVelocity.x = 0.f;
									physics->SetVelocity(newVelocity);
								}
							}
						}
						else
						{
							currentVirus.timerHit -= dt;
							if (currentVirus.timerHit <= 0.f)
							{
								currentVirus.timerHit = 5.f;
								currentVirus.hasBeenHit = false;
								currentVirus.returningToX = true;
								vec3 currentVelocity = physics->GetVelocity();
								currentVelocity.x = 0.f;
								physics->SetVelocity(currentVelocity);
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

	AddDebugText("camera.finalPosition: " + VecToString(camera.GetPlainPosition()));
	AddDebugText("player.physics.postion: " + VecToString(player.renderGroup.lock()->GetPhysics()->GetPosition()));
	AddDebugText("player.physics.velocity: " + VecToString(player.renderGroup.lock()->GetPhysics()->GetVelocity()));
	AddDebugText("gaugeMarker.position: " + VecToString(gaugeMarkerPosition));

	if (dirtyWorldList) {
		for (unsigned i = 0; i < worldList.size(); ) {
			if (worldList[i].expired()) {
				worldList.erase(worldList.begin() + i);
				continue;
			}
			i++;
		}
	}

	if (inGame && !endGame)
	{
		if (allGameObjectsDestroyed)
		{
			if (AudioManager::GetInstance().PlayingMUS() == 1)
			{
				AudioManager::GetInstance().FadeOutMUS(1000);
			}
			endGame = true;
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
			if (!pauseMarker)
			{
				pauseMarker = true;
			}
			else
			{
				pauseMarker = false;
			}
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_6))
		{
			gameTimer = 0.f;
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_A))
		{
			portalIndex--;
			if (portalIndex < 0)
			{
				portalIndex = portalGoodPositionsList.size() - 1;
			}
		}

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_D))
		{
			portalIndex++;
			if (portalIndex % portalGoodPositionsList.size() == 0 && portalIndex != 0)
			{
				portalIndex = 0;
			}
		}
		if (!portalGoodList.empty()) selectedPortalPosition = portalGoodList[portalIndex].portalBasePosition;

		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_7))
		{
			triggeredChange = true;
		}

		if (triggeredChange)
		{
			portalIndex = 0;
		}

		if (MouseController::GetInstance()->IsButtonDown(MouseController::LMB) && !endGame) {
			isGaugeActive = true;
		}
		else
		{
			isGaugeActive = false;
		}
		if (MouseController::GetInstance()->IsButtonReleased(MouseController::LMB) && !endGame)
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
			if (gaugeMarkerPosition.x < 0.03f && gaugeMarkerPosition.x > -0.03f) // Green
			{
				impulseStrength = 50000.f;
			}
			else if (gaugeMarkerPosition.x < 0.1f && gaugeMarkerPosition.x > -0.1f) // Yellow
			{
				impulseStrength = 35000.f;
			}
			else if (gaugeMarkerPosition.x < 0.165f && gaugeMarkerPosition.x > -0.165f) // Orange
			{
				impulseStrength = 25000.f;
			}
			else if (gaugeMarkerPosition.x < 0.225f && gaugeMarkerPosition.x > -0.225f) // Red
			{
				impulseStrength = 20000.f;
			}

			physics->AddImpulse(impulseStrength * direction);
			physics->AddTorque(vec3(5.f, 30.f, 0.f));
		}


		break;
	case FREEROAM:
		if (player.allowControl)
		{
			if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_Z))
			{
				if (canStartGame)
				{
					gameIntroActive = true;
					currentGameState = DIALOGUE;
					canStartGame = false;
				}
			}

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

void SceneWhack::MidgameChange()
{
	gameChange = true;
	auto& physicsList = RObj::physicsList;
	bool allDisappeared = true;
	for (unsigned i = 0; i < physicsList.size();)
	{
		if (physicsList[i].expired()) {
			physicsList.erase(physicsList.begin() + i);
			continue;
		}

		auto obj = physicsList[i].lock();
		auto physics = obj->GetPhysics();

		if (obj->name == "portal_good" || obj->name == "portal_evil")
		{
			if (obj->alpha > 0.f)
			{
				return;
			}
		}

		i++;
	}

	triggeredChange = false;
	CreatePortalMap();
}

void SceneWhack::InitPortalMaps()
{
	portalEvilMapList[0] = { vec3(40.f, 15.f, -5.f), vec3(40.f, 15.f, 5.f), vec3(40.f, 5.f, -5.f), vec3(40.f, 5.f, 5.f) };
	portalEvilMapList[1] = { vec3(40.f, 15.f, 0.f), vec3(40.f, 10.f, -9.f), vec3(40.f, 10.f, 9.f),  vec3(40.f, 5.f, 0.f) };

	portalGoodMapList[0] = { vec3(43.f, 15.f, 0.f), vec3(43.f, 10.f, -5.f), vec3(43.f, 10.f, 0.f), vec3(43.f, 10.f, 5.f),  vec3(43.f, 5.f, 0.f) };
	portalGoodMapList[1] = { vec3(43.f, 13.f, -5.f), vec3(43.f, 13.f, 5.f), vec3(43.f, 10.f, 0.f), vec3(43.f, 7.f, -5.f), vec3(43.f, 7.f, 5.f) };
}

void SceneWhack::CreatePortalMap()
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
		newObj->alpha = 0.f;
		newObj->GetPhysics()->SetPosition(portalEvilPositionsList[i]);
		newObj->colorFilter = vec3(1.f, 0.1f, 0.1f);

		portalEvilList.push_back({ portalEvilPositionsList[i], newObj });
	}

	for (int i = 0; i < portalGoodPositionsList.size(); i++)
	{
		worldRoot->NewChild(MeshObject::Create(PORTAL_GOOD));
		newObj->name = "portal_good";
		newObj->alpha = 0.f;
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
	gameChange = false;
	triggeredChange = false;
}

void SceneWhack::StartCutscene()
{
	if (DialogueManager::GetInstance().CheckActivePack()) return;
	static int cutsceneIndex = 0;

	// maybe add enums for each cutscene later
	if (!hasFinishedTutorial)
	{
		currentGameState = DIALOGUE;
		switch (cutsceneIndex)
		{
		case 0:
			player.allowControl = false;
			hasFinishedCarnivalIntro = false;
			camera.Set(FPCamera::MODE::LOCK_ON);
			camera.SetDirection(vec3(20.f, 4.f, 0.f) - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("CarnivalIntro");
			cutsceneIndex++;
			break;
		case 1:
			hasFinishedCarnivalIntro = true;
			DialogueManager::GetInstance().StartDialogue("Tutorial1");
			cutsceneIndex++;
			break;
		case 2:
			showTutorial = true;
			DialogueManager::GetInstance().StartDialogue("Tutorial2");
			cutsceneIndex++;
			break;
		case 3:
			showTutorial = false;
			showScoreDisplay = true;
			camera.SetDirection(scoreDisplayPosition - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("Tutorial3");
			cutsceneIndex++;
			break;
		case 4:
			showScoreDisplay = false;
			camera.SetDirection(vec3(7.f, 6.f, -7.f) - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("Tutorial4");
			cutsceneIndex++;
			break;
		case 5:
			camera.SetDirection(vec3(20.f, 4.f, 0.f) - camera.GetPlainPosition());
			DialogueManager::GetInstance().StartDialogue("Tutorial5");
			cutsceneIndex++;
			break;
		case 6:
			if (DialogueManager::GetInstance().CheckActivePack()) return;
			hasFinishedTutorial = true;
			camera.Set(Cam::MODE::FIRST_PERSON);
			player.allowControl = true;
			currentGameState = FREEROAM;
			cutsceneIndex = 0;
			break;
		default:
			break;
		}
	}

	if (gameIntroActive)
	{
		currentGameState = DIALOGUE;
		switch (cutsceneIndex)
		{
		case 0:
			player.renderGroup.lock()->GetPhysics()->Getbody()->enableGravity(false);
			player.renderGroup.lock()->GetPhysics()->SetVelocity(vec3(player.renderGroup.lock()->GetPhysics()->GetVelocity().x, 0.f, player.renderGroup.lock()->GetPhysics()->GetVelocity().z));
			camera.Set(Cam::MODE::LOCK_ON);
			player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(15.f, 10.f, 0.f));
			camera.SetDirection(vec3(20.f, 6.5f, 0.f) - vec3(10.f, 6.5f, 0.f));
			DialogueManager::GetInstance().StartDialogue("GameIntro");
			cutsceneIndex++;
			break;
		case 1:
			isUpdateGameLight = true;
			cutsceneIndex++;
			break;
		case 2:
			gameIntroActive = false;
			inGame = true;
			isNewRecord = false;
			gameScore = 0;
			currentGameState = GAMEPLAY;
			gameTimer = 60.f;
			AudioManager::GetInstance().PlayMUS(-1, 0);
			CreatePortalMap();
			elapsedVirusInterval = 0;
			cutsceneIndex = 0;
			break;
		default:
			break;
		}
	}

	if (endGame)
	{
		currentGameState = DIALOGUE;
		switch (cutsceneIndex)
		{
		case 0:
			isUpdateGameLight = true;
			if (gameScore > highestScore)
			{
				highestScore = gameScore;
				AudioManager::GetInstance().PlaySFX(NEWRECORD);
				DialogueManager::GetInstance().StartDialogue("EndGame_NewRecord");
			}
			else if (gameScore == 0)
			{
				AudioManager::GetInstance().PlaySFX(ZEROSCORE);
				DialogueManager::GetInstance().StartDialogue("EndGame_ZeroScore");
			}
			else
			{
				AudioManager::GetInstance().PlaySFX(OKAYSCORE);
				DialogueManager::GetInstance().StartDialogue("EndGame");
			}
			cutsceneIndex++;
			break;
		case 1:
			currentGameState = FREEROAM;
			player.renderGroup.lock()->GetPhysics()->Getbody()->enableGravity(true);
			inGame = false;
			allGameObjectsDestroyed = false;
			camera.Set(Cam::MODE::FIRST_PERSON);
			player.renderGroup.lock()->GetPhysics()->SetPosition(vec3(0.f, 6.5f, 0.f));
			cutsceneIndex = 0;
			endGame = false;
			break;
		default:
			break;
		}
	}
}
