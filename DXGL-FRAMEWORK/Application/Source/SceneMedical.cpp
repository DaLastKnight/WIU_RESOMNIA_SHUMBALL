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
			PhysicsManager::GetInstance().SetUpLogger("SceneMedical");
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





		// ***************************************************************
		// SceneMedical GEOMETRY List
		// ***************************************************************
		meshList[ENV_SKYBOX] = MeshBuilder::GenerateSkybox("Map Boundary", TextureLoader::LoadTexture("internal_body.png"));
		meshList[ENV_SPHERE_MODEL] = MeshBuilder::GenerateSphere("Map Sphered Environment", vec3(1));
		meshList[ENV_SPHERE_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_BLOCK_MODEL] = MeshBuilder::GenerateCube("Map Block Environment", vec3(1), 1);
		meshList[ENV_BLOCK_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_STRING_MODEL] = MeshBuilder::GenerateCylinder("Map String Environment", vec3(1), 360, 0.5f, 1);
		meshList[ENV_STRING_MODEL]->textureID = TextureLoader::LoadTexture("internal_body.png");
		meshList[ENV_LIQUID_MODEL] = MeshBuilder::GenerateCylinder("Map Liquid Environment", vec3(1), 360, 0.5f, 1);
		meshList[ENV_LIQUID_MODEL]->textureID = TextureLoader::LoadTexture("red_liquid.png");
		meshList[ENV_LIQUID_FLAT_MODEL] = MeshBuilder::GenerateQuad("Map Liquid Flat Environment", vec3(1), 1, 1, TextureLoader::LoadTexture("red_liquid.png"));
		
		meshList[BACTERIA_MODEL] = MeshBuilder::GenerateOBJMTL("Bacteria", "bacteria_optimised.obj", "bacteria_optimised.mtl", TextureLoader::LoadTexture("bacteria_skin.png"));
		meshList[VIRUS_MODEL] = MeshBuilder::GenerateOBJMTL("Virus", "bacteria_optimised.obj", "bacteria_optimised.mtl", TextureLoader::LoadTexture("virus_skin.png"));
		meshList[NANOBOT_MODEL] = MeshBuilder::GenerateOBJMTL("Nanobot", "nanobot.obj", "nanobot.mtl", TextureLoader::LoadTexture("nanobot_skin.png"));
		
		meshList[GAME_CROSSHAIR] = MeshBuilder::GenerateQuad("Crosshair", vec3(1, 1, 0), 1, 1);
		meshList[GAME_UI_BASE] = MeshBuilder::GenerateQuad("UI Base", vec3(0.7f, 0.7f, 0.7f), 1, 1);
		meshList[GAME_UI_PLATE] = MeshBuilder::GenerateQuad("UI Plating", vec3(0), 1, 1);
		meshList[GAME_OVERLOADSTACK_G] = MeshBuilder::GenerateQuad("Overload UI Safe", vec3(0, 1, 0), 1, 1);
		meshList[GAME_OVERLOADSTACK_Y] = MeshBuilder::GenerateQuad("Overload UI Warning", vec3(1, 1, 0), 1, 1);
		meshList[GAME_OVERLOADSTACK_R] = MeshBuilder::GenerateQuad("Overload UI Danger", vec3(1, 0, 0), 1, 1);
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

		RObj::worldList.reserve(200); // Save 200 spaces in the world space to render 200 objects?
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





		// ***************************************************************
		// SceneMedical Specific DefaultStat Settings
		// ***************************************************************
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
			});

		RObj::setDefaultStat.Subscribe(BACTERIA_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(VIRUS_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});
		RObj::setDefaultStat.Subscribe(NANOBOT_MODEL, [](const std::shared_ptr<RObj>& obj) {
			});

		RObj::setDefaultStat.Subscribe(GAME_CROSSHAIR, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAME_UI_BASE, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAME_UI_PLATE, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAME_OVERLOADSTACK_G, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAME_OVERLOADSTACK_Y, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
		RObj::setDefaultStat.Subscribe(GAME_OVERLOADSTACK_R, [](const std::shared_ptr<RObj>& obj) {
			obj->relativeTrl = true;
			obj->hasTransparency = true;
			});
	}

	auto& newObj = RObj::newObject;

	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(AXES));

		worldRoot->NewChild(MeshObject::Create(SKYBOX));




		
		// ***************************************************************
		// SceneMedical Specific Playing Skybox (Fallback)
		// ***************************************************************
		worldRoot->NewChild(MeshObject::Create(ENV_SKYBOX));
		newObj->scl = vec3(0.1f, 0.1f, 0.1f);





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
		}





		// ***************************************************************
		// Map Boundaries
		// ***************************************************************
		{
			worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Left
			newObj->trl = glm::vec3(-99.9f, 0, 0);
			newObj->offsetScl = glm::vec3(100, 100, 100);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(50, 50, 50), vec3(0, 0, 0));
			physics->SetPosition(newObj->trl);
		}
		{
			worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Right
			newObj->trl = glm::vec3(99.9f, 0, 0);
			newObj->offsetScl = glm::vec3(100, 100, 100);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(50, 50, 50), vec3(0, 0, 0));
			physics->SetPosition(newObj->trl);
		}
		{
			worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Back
			newObj->trl = glm::vec3(0, 0, -99.9f);
			newObj->offsetScl = glm::vec3(100, 100, 100);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(50, 50, 50), vec3(0, 0, 0));
			physics->SetPosition(newObj->trl);
		}
		{
			worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Front
			newObj->trl = glm::vec3(0, 0, 99.9f);
			newObj->offsetScl = glm::vec3(100, 100, 100);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(50, 50, 50), vec3(0, 0, 0));
			physics->SetPosition(newObj->trl);
		}





		// ***************************************************************
		// Sphere Map Designs (unlikely interactable)
		// ***************************************************************
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





		// ***************************************************************
		// Top-Down Map Designs (More likely interactable)
		// ***************************************************************
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(0, -30, 50);
		newObj->scl = glm::vec3(30, 30, 10);
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(40, -45, 30);
		newObj->scl = glm::vec3(40, 10, 40);
		{
			worldRoot->NewChild(MeshObject::Create(ENV_STRING_MODEL)); // Needs collisions
			
			newObj->trl = glm::vec3(45, 0, 45);
			newObj->offsetScl = glm::vec3(20, 100, 20);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(10, 50, 10), vec3(0, 0, 0));
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
			
			newObj->trl = glm::vec3(-50, 0, 10);
			newObj->offsetScl = glm::vec3(20, 100, 20);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(10, 50, 10), vec3(0, 0, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->SetPosition(newObj->trl);
		}
		worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL));
		newObj->trl = glm::vec3(-50, 40, -40);
		newObj->scl = glm::vec3(30, 30, 40);
		{
			worldRoot->NewChild(MeshObject::Create(ENV_BLOCK_MODEL)); // Probably needs collision
			
			newObj->trl = glm::vec3(0, 0, -50);
			newObj->offsetScl = glm::vec3(30, 100, 10);

			newObj->AddPhysics(PhysicsObject::STATIC);
			auto physics = newObj->GetPhysics();

			physics->AddCollider(PhysicsObject::BOX, vec3(15, 50, 5), vec3(0, 0, 0));
			physics->SetBounciness(0.f);
			physics->SetFrictionCoefficient(0.5f);
			physics->SetPosition(newObj->trl);
			physics->SetCollisionActive(true);
		}





		// ***************************************************************
		// Fluid Designs
		// ***************************************************************
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_MODEL));
		newObj->trl = glm::vec3(10, 0, -5);
		newObj->scl = glm::vec3(5, 100, 5);
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_MODEL));
		newObj->trl = glm::vec3(-30, 0, -20);
		newObj->scl = glm::vec3(5, 100, 5);
		worldRoot->NewChild(MeshObject::Create(ENV_LIQUID_FLAT_MODEL));
		newObj->trl = glm::vec3(0, -49, 0);
		newObj->rot = glm::vec3(-90, 0, 0);
		newObj->scl = glm::vec3(100, 100, 1);




		
		// ***************************************************************
		// Strings Map Design
		// ***************************************************************
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
			newObj->scl = glm::vec3(0.8f, 100.f, 0.8f);
		}
	}

	// view space init
	{
		// ***************************************************************
		// Player Nanobot Representation
		// ***************************************************************
		viewRoot->NewChild(MeshObject::Create(NANOBOT_MODEL));
		newObj->trl = glm::vec3(0.0f, -0.2f, -0.5f);
		newObj->scl = glm::vec3(0.1f, 0.1f, 0.1f);
	}

	// screen space init
	{
		// ***************************************************************
		// Crosshair UI
		// ***************************************************************
		screenRoot->NewChild(MeshObject::Create(GAME_CROSSHAIR)); // Vertical
		newObj->scl = glm::vec3(5.f, 20.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_CROSSHAIR)); // Horizontal
		newObj->scl = glm::vec3(20.f, 5.f, 1);





		// ***************************************************************
		// Overload Mechanic UI
		// ***************************************************************
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE));
		newObj->trl = glm::vec3(0.75f, -0.9f, 0);
		newObj->scl = glm::vec3(350.f, 50.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_PLATE, 1));
		newObj->trl = glm::vec3(0.75f, -0.9f, 0);
		newObj->scl = glm::vec3(340.f, 40.f, 1);





		// ***************************************************************
		// Pause Button UI
		// ***************************************************************
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE));
		newObj->trl = glm::vec3(0.925f, 0.875f, 0);
		newObj->scl = glm::vec3(80.f, 80.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_PLATE, 1));
		newObj->trl = glm::vec3(0.925f, 0.875f, 0);
		newObj->scl = glm::vec3(70.f, 70.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE, 2));
		newObj->trl = glm::vec3(0.91f, 0.875f, 0);
		newObj->scl = glm::vec3(10.f, 40.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE, 2));
		newObj->trl = glm::vec3(0.94f, 0.875f, 0);
		newObj->scl = glm::vec3(10.f, 40.f, 1);





		// ***************************************************************
		// Help Button UI
		// ***************************************************************
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE));
		newObj->trl = glm::vec3(0.825f, 0.875f, 0);
		newObj->scl = glm::vec3(70.f, 70.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_PLATE, 1));
		newObj->trl = glm::vec3(0.825f, 0.875f, 0);
		newObj->scl = glm::vec3(60.f, 60.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE, 2));
		newObj->trl = glm::vec3(0.825f, 0.89f, 0);
		newObj->scl = glm::vec3(10.f, 25.f, 1);
		screenRoot->NewChild(MeshObject::Create(GAME_UI_BASE, 2));
		newObj->trl = glm::vec3(0.825f, 0.84f, 0);
		newObj->scl = glm::vec3(10.f, 10.f, 1);





		// ***************************************************************
		// Texts
		// ***************************************************************
		InitDebugText(FONT_CASCADIA_MONO); // if you want another font for debug text, just change it to another font, tho dont call this in Update(), itll break
		InitSceneMedicalText(FONT_CASCADIA_MONO);
	}

	/************************ below for external class inits ************************/
	{
		// camera init
		camera.Init(glm::vec3(1, 2, -1), glm::vec3(-1, -1, 1));
		camera.Set(Cam::MODE::FIRST_PERSON);

		// player init
		player.Init(worldRoot, GROUP, vec3(0, 2, 0));
	}

	RObj::newObject.reset();
}

void SceneMedical::Update(double dt) 
{
	BaseScene::Update(dt);

	ClearDebugText();
	ClearSceneMedicalText();

	auto& cameraMode = camera.GetCurrentMode();

	



	// ***********************************************************
	// Win Condition Management for SceneMedical
	// ***********************************************************
	if (currentState == MedicalGameState::WON && !isInResults)
	{
		isInResults = true;
		isGameReset = true;
		currentState = MedicalGameState::RESULTS;
		HandleWinCondition();
	}
	if (currentState == MedicalGameState::RESULTS)
	{
		player.allowControl = false;
		camera.Set(Cam::MODE::PAUSE);
	}





	// ********************************************************************
	// Lose Condition Management for SceneMedical (Checkpoint Reset)
	// ********************************************************************
	if (overloadingStack >= 6 || waveTimeLeft <= 0.0f)
	{
		ChangeWave(waveNumber); // Reset Wave Data
		// Delete all previous entities
		for (int i = static_cast<int>(nanobots.size()) - 1; i >= 0; --i)
		{
			auto& delNano = nanobots[i];

			if (!delNano.object || !delNano.object->GetPhysics())
			{
				continue;
			}

			delNano.object->Destroy();
			nanobots.erase(nanobots.begin() + i);
		}
		for (int i = static_cast<int>(bacterias.size()) - 1; i >= 0; --i)
		{
			auto& delBact = bacterias[i];

			if (!delBact.object || !delBact.object->GetPhysics())
			{
				continue;
			}

			delBact.object->Destroy();
			bacterias.erase(bacterias.begin() + i);
		}
		for (int i = static_cast<int>(viruses.size()) - 1; i >= 0; --i)
		{
			auto& delVir = viruses[i];

			if (!delVir.object || !delVir.object->GetPhysics())
			{
				continue;
			}

			delVir.object->Destroy();
			viruses.erase(viruses.begin() + i);
		}

		currentActiveNanobotAmmo = 0;
		changeInOverloadStack = true;
		overloadingStack = 0;
	}





	// *********************************************************************
	// Reset/Retry Game After Victory Management for SceneMedical
	// *********************************************************************
	if (isGameReset)
	{
		ChangeWave(1); // Reset Wave Data

		// Delete all previous entities
		for (int i = static_cast<int>(nanobots.size()) - 1; i >= 0; --i)
		{
			auto& delNano = nanobots[i];

			if (!delNano.object || !delNano.object->GetPhysics())
			{
				continue;
			}

			delNano.object->Destroy();
			nanobots.erase(nanobots.begin() + i);
		}
		for (int i = static_cast<int>(bacterias.size()) - 1; i >= 0; --i)
		{
			auto& delBact = bacterias[i];

			if (!delBact.object || !delBact.object->GetPhysics())
			{
				continue;
			}

			delBact.object->Destroy();
			bacterias.erase(bacterias.begin() + i);
		}
		for (int i = static_cast<int>(viruses.size()) - 1; i >= 0; --i)
		{
			auto& delVir = viruses[i];

			if (!delVir.object || !delVir.object->GetPhysics())
			{
				continue;
			}

			delVir.object->Destroy();
			viruses.erase(viruses.begin() + i);
		}
		
		currentActiveNanobotAmmo = 0;
		changeInOverloadStack = true;
		overloadingStack = 0;

		totalTimeTaken = 0;

		isGameReset = false;
	}

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





	// ****************************************************************
	// Game Timer Management for SceneMedical
	// ****************************************************************
	waveTimeAccumulator += dt;

	if (waveTimeAccumulator >= 1.0f)
	{
		waveTimeAccumulator -= 1.0f;
		if (cameraMode == Cam::MODE::PAUSE)
		{
			waveTimeLeft = waveTimeLeft;
			totalTimeTaken = totalTimeTaken;
		}
		else
		{
			waveTimeLeft--;
			totalTimeTaken++;
		}

		if (waveTimeLeft <= 0.0f)
		{
			waveTimeLeft = 180;
		}
	}

	if (cameraMode != Cam::MODE::PAUSE)
	{
		bacteriaSpawnTimer += dt;
		virusSpawnTimer += dt;
		if (overloadingStack > 3)
		{
			overloadCoolTimer += dt;
			if (overloadCoolTimer >= 10.0f)
			{
				overloadCoolTimer = 10.0f;
			}
		}
		for (int i = static_cast<int>(nanobots.size()) - 1; i >= 0; --i)
		{
			nanobots[i].lifetime -= dt;
		}
		for (int i = static_cast<int>(bacterias.size()) - 1; i >= 0; --i)
		{
			bacterias[i].bacteriaDivertTimer -= dt;
			if (bacterias[i].bacteriaInvulnerabilityTimer > 0.0f)
			{
				bacterias[i].bacteriaInvulnerabilityTimer -= dt;
				if (bacterias[i].bacteriaInvulnerabilityTimer < 0.0f)
				{
					bacterias[i].bacteriaInvulnerabilityTimer = 0.0f;
				}
			}
		}
		for (int i = static_cast<int>(viruses.size()) - 1; i >= 0; --i)
		{
			if (viruses[i].virusInvulnerabilityTimer > 0.0f)
			{
				viruses[i].virusInvulnerabilityTimer -= dt;
				if (viruses[i].virusInvulnerabilityTimer < 0.0f)
				{
					viruses[i].virusInvulnerabilityTimer = 0.0f;
				}
			}
		}
	}





	// ***************************************************************************
	// Wave Data Display and In-Scene Management
	// ***************************************************************************
	if (waveNumber == 1 && remainingEntitiesP <= 0 && remainingEntitiesAI <= 0)
	{
		ChangeWave(2);
	}
	else if (waveNumber == 2 && remainingEntitiesP <= 0 && remainingEntitiesAI <= 0)
	{
		ChangeWave(3);
	}
	else if (waveNumber == 3 && remainingEntitiesP <= 0 && remainingEntitiesAI <= 0 && isGameReset == false)
	{
		currentState = MedicalGameState::WON;
	}

	AddSceneMedicalText("Wave: " + std::to_string(waveNumber) + "/3");
	AddSceneMedicalText("Time Left: " + std::to_string(waveTimeLeft) + "s");
	AddSceneMedicalText("--------------------");
	AddSceneMedicalText("Bacteria Alive: " + std::to_string(remainingEntitiesP) + "/" + std::to_string(maxEntitiesP));
	AddSceneMedicalText("Viruses Alive: " + std::to_string(remainingEntitiesAI) + "/" + std::to_string(maxEntitiesAI));
	AddSceneMedicalText("Nanobot Ammo: " + std::to_string(maxNanobotAmmo - currentActiveNanobotAmmo) + "/" + std::to_string(maxNanobotAmmo));
	AddSceneMedicalText("--------------------");
	AddSceneMedicalText("Total Time: " + std::to_string(totalTimeTaken) + "s");





	// *****************************************************************
	// Overload Cooling Mechanic Management for SceneMedical
	// *****************************************************************
	if (overloadingStack > 3)
	{
		if (overloadCoolTimer >= 10.0f)
		{
			overloadingStack--;
			changeInOverloadStack = true;
			overloadCoolTimer = 0.0f;
		}
	}





	// Temporary for now
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

	HandleKeyPress();

	// player updates
	{
		// update position and camera bobbing
		if (camera.GetCurrentMode() != Cam::MODE::FREE)
			player.UpdatePhysicsWithCamera(dt, camera);
		else
			player.UpdatePhysics(dt);
	}

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
		//if (obj->name == "demo light spot") {
		//	obj->offsetRot.y += 45 * dt;
		//	obj->isDirty = true; // UpdateModel() cannot detect changes in offsets, so you need to manually set isDirty to true
		//} // tho normally you wont need to touch offsets in Update() at all since you normally will have a group obj that is parented to this





		// ***************************************************************************
		// Nanobot Projectile Spawning and Movement Updates | Straight Line Shot Type
		// ***************************************************************************
		if (isNanobotFired)
		{
			currentActiveNanobotAmmo++;

			worldRoot->NewChild(MeshObject::Create(NANOBOT_MODEL));

			std::shared_ptr<RenderObject> nanobot = RenderObject::newObject;

			glm::vec3 travelDir = camera.GetPlainDirection();
			float projectileSpeed = 200000.0f;

			nanobot->offsetRot = glm::vec3(0.f, 90.f, 0.f);
			nanobot->offsetScl = glm::vec3(0.5f, 0.5f, 0.5f);

			nanobot->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = nanobot->GetPhysics();
			physics->SetPosition(camera.GetPlainPosition() + travelDir * 1.5f);

			physics->AddCollider(PhysicsObject::SPHERE, vec3(0.5f), vec3(0));

			glm::vec3 finalForce = travelDir * projectileSpeed;

			if (cameraMode != Cam::MODE::PAUSE)
			{
				physics->AddImpulse(finalForce);
			}

			Nanobot nb;
			nb.object = nanobot;
			nb.lifetime = 5.0f;

			nanobots.push_back(nb);

			isNanobotFired = false;
		}

		for (int i = static_cast<int>(nanobots.size()) - 1; i >= 0; --i)
		{
			if (nanobots[i].lifetime <= 0.0f)
			{
			    nanobots[i].object->Destroy(); // Remove Physics and Visual Body, struct already destroyed with the shared_ptr so no reset needed
				nanobots.erase(nanobots.begin() + i); // Remove from container
				currentActiveNanobotAmmo--;
			}
		}





		// ****************************************************************************************************
		// Enemy: Bacteria | Spawning, Movement and Collision Updates for SceneMedical | Waypoint Patrol Type
		// ****************************************************************************************************
		if (bacteriaSpawnTimer >= bacteriaSpawnInterval && currentSpawningP < maxEntitiesP)
		{
			bacteriaSpawnTimer = 0.0f;

			worldRoot->NewChild(MeshObject::Create(BACTERIA_MODEL));

			std::shared_ptr<RenderObject> bacteria = RenderObject::newObject;

			float posX = static_cast<float>(rand() % 81 - 40);
			float posY = static_cast<float>(rand() % 21 - 10);
			float posZ = static_cast<float>(rand() % 81 - 40);

			while (glm::vec3(posX, posY, posZ) == camera.GetPlainPosition()) // Prevent spawning on player
			{
				posX = static_cast<float>(rand() % 81 - 40);
				posY = static_cast<float>(rand() % 21 - 10);
				posZ = static_cast<float>(rand() % 81 - 40);
			}

			bacteria->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = bacteria->GetPhysics();

			physics->AddCollider(PhysicsObject::SPHERE, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, 0, 0));

			physics->SetPosition(glm::vec3(posX, posY, posZ));

			Bacteria b;
			b.object = bacteria;
			b.bacteriaDivertTimer = 0.0f;
			b.bacteriaHP = 2;
			b.bacteriaInvulnerabilityTimer = 0.0f;
			bacterias.push_back(b);

			currentSpawningP++;
		}

		for (int i = static_cast<int>(bacterias.size()) - 1; i >= 0; --i)
		{
			auto& bacteria = bacterias[i]; // re-refer to inside the container

			if (!bacteria.object || !bacteria.object->GetPhysics())
			{
				continue;
			}

			auto physics = bacteria.object->GetPhysics();

			if (bacteria.bacteriaDivertTimer <= 0.0f)
			{
				bacteria.patPoint.x = static_cast<float>(rand() % 81 - 40);
				bacteria.patPoint.z = static_cast<float>(rand() % 81 - 40);

				if (waveNumber == 1)
				{
					bacteria.bacteriaDivertTimer = 3.0f; // Reset timer so that it moves 3 seconds later
				}
				else
				{
					bacteria.bacteriaDivertTimer = 2.0f;
				}
			}

			glm::vec3 phyPos = physics->GetPosition();
			glm::vec3 patDir = glm::vec3(bacteria.patPoint.x, phyPos.y, bacteria.patPoint.z) - phyPos;

			bool bacteriaDestroyed = false;

			float distance = glm::length(patDir);

			if (distance > 0.1f)
			{
				patDir = glm::normalize(patDir);
				float bacteriaMovementSpeed = 0.0f;

				if (waveNumber == 1)
				{
					bacteriaMovementSpeed = 4.0f;
				}
				else if (waveNumber == 2)
				{
					bacteriaMovementSpeed = 5.0f;
				}
				else
				{
					bacteriaMovementSpeed = 6.0f;
				}

				glm::vec3 finalVel = patDir * bacteriaMovementSpeed;

				if (cameraMode != Cam::MODE::PAUSE)
				{
					physics->SetVelocity(finalVel);
				}
			}

			glm::vec3 dToPDir = camera.GetPlainPosition() - phyPos;
			float distanceToPlayer = glm::length(dToPDir);

			if (distanceToPlayer <= 2.0f + 2.0f && cameraMode != Cam::MODE::PAUSE) // Ignore collider with player by expanding fake hitbox check
			{
				bacteria.object->Destroy();
				bacterias.erase(bacterias.begin() + i); // Remove the exact element

				overloadingStack++;
				changeInOverloadStack = true;
				remainingEntitiesP--;
			}

			for (int j = static_cast<int>(nanobots.size()) - 1; j >= 0 && !bacteriaDestroyed; --j)
			{
				if (!nanobots[j].object || !nanobots[j].object->GetPhysics())
				{
					continue;
				}

				auto nanoPhy = nanobots[j].object->GetPhysics();
				glm::vec3 dToNanoDir = nanoPhy->GetPosition() - phyPos;

				float distanceToNano = glm::length(dToNanoDir);

				if (distanceToNano <= 1.0f + 1.0f)
				{
					if (bacteria.bacteriaHP > 0 && bacteria.bacteriaInvulnerabilityTimer <= 0.0f)
					{
						bacteria.bacteriaHP--;
						bacteria.bacteriaInvulnerabilityTimer = 1.0f;
					}
					

					if (bacteria.bacteriaHP <= 0)
					{
						bacteria.object->Destroy();
						bacterias.erase(bacterias.begin() + i); // Remove the exact element

						remainingEntitiesP--;

						bacteriaDestroyed = true;
					}
				}
			}	
		}





		// ****************************************************************************************************
		// Enemy: Virus | Spawning, Movement and Collision Updates for SceneMedical | AI Move To Player Type
		// ****************************************************************************************************
		if (virusSpawnTimer >= virusSpawnInterval && currentSpawningAI < maxEntitiesAI)
		{
			virusSpawnTimer = 0.0f;

			worldRoot->NewChild(MeshObject::Create(VIRUS_MODEL));

			std::shared_ptr<RenderObject> virus = RenderObject::newObject;

			float posX = static_cast<float>(rand() % 81 - 40);
			float posY = static_cast<float>(rand() % 21 - 10);
			float posZ = static_cast<float>(rand() % 81 - 40);

			while (glm::vec3(posX, posY, posZ) == camera.GetPlainPosition()) // Prevent spawning on player
			{
				posX = static_cast<float>(rand() % 81 - 40);
				posY = static_cast<float>(rand() % 21 - 10);
				posZ = static_cast<float>(rand() % 81 - 40);
			}

			virus->AddPhysics(PhysicsObject::DYNAMIC);
			auto physics = virus->GetPhysics();

			physics->AddCollider(PhysicsObject::SPHERE, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, 0, 0));

			physics->SetPosition(glm::vec3(posX, posY, posZ));

			Virus v;
			v.object = virus;
			v.virusHP = 3;
			v.virusInvulnerabilityTimer = 0.0f;
			viruses.push_back(v); // Send to container

			currentSpawningAI++;
		}

		for (int i = static_cast<int>(viruses.size()) - 1; i >= 0; --i)
		{
			auto& virus = viruses[i];

			if (!virus.object || !virus.object->GetPhysics())
			{
				continue;
			}

			auto physics = virus.object->GetPhysics();
			glm::vec3 phyPos = physics->GetPosition();
			glm::vec3 AIDir = camera.GetPlainPosition() - phyPos;

			bool virusDestroyed = false;

			float distance = glm::length(AIDir);

			if (distance > 0.1f)
			{
				AIDir = glm::normalize(AIDir);
				float virusMovementSpeed = 0.0f;

				if (waveNumber == 1)
				{
					virusMovementSpeed = 3.0f;
				}
				else if (waveNumber == 2)
				{
					virusMovementSpeed = 4.0f;
				}
				else
				{
					virusMovementSpeed = 5.0f;
				}

				glm::vec3 finalVel = AIDir * virusMovementSpeed;

				if (cameraMode != Cam::MODE::PAUSE)
				{
					physics->SetVelocity(finalVel);
				}
			}

			if (distance <= 2.0f + 2.0f && cameraMode != Cam::MODE::PAUSE)
			{
				virus.object->Destroy();
				viruses.erase(viruses.begin() + i); // Remove the exact element

				overloadingStack++;
				changeInOverloadStack = true;
				remainingEntitiesAI--;
			}

			for (int j = static_cast<int>(nanobots.size()) - 1; j >= 0 && !virusDestroyed; --j)
			{
				if (!nanobots[j].object || !nanobots[j].object->GetPhysics())
				{
					continue;
				}

				auto nanoPhy = nanobots[j].object->GetPhysics();
				glm::vec3 dToNanoDir = nanoPhy->GetPosition() - phyPos;

				float distanceToNano = glm::length(dToNanoDir);

				if (distanceToNano <= 1.0f + 1.0f)
				{
					if (virus.virusHP > 0 && virus.virusInvulnerabilityTimer <= 0.0f)
					{
						virus.virusHP--;
						virus.virusInvulnerabilityTimer = 1.0f;
					}

					if (virus.virusHP <= 0)
					{
						virus.object->Destroy();
						viruses.erase(viruses.begin() + i); // Remove the exact element

						remainingEntitiesAI--;

						virusDestroyed = true; // exit the loop and prevent accessing erased memory for the rest of the loop as object is already gone
					}
				}
			}
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





		// ***************************************************************************
		// Overload Mechanic Visual Updates for SceneMedical
		// ***************************************************************************
		if (changeInOverloadStack)
		{
			ShowOverloadStack();
			changeInOverloadStack = false;
		}





		// *****************************************************************************
		// Menu Object Visual Updates for SceneMedical
		// *****************************************************************************
		if (isHelpOpen && menuChange && currentState == MedicalGameState::PLAYING)
		{
			ShowHelpMenu();
			menuChange = false;
		}

		if (!isHelpOpen && menuChange && currentState == MedicalGameState::PLAYING)
		{
			ClearHelpMenu();
			menuChange = false;
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
			if (!info.obj)
			{
				continue;
			}
			modelStack.PushMatrix();
			modelStack.LoadMatrix(info.model);
			RenderObj(info.obj);
			modelStack.PopMatrix();
		}
		};

	auto renderObjectList = [&](const std::vector<std::weak_ptr<RObj>>& list, bool ignoreTransparency = false) {
		for (auto& obj_wptr : list) {
			auto obj = obj_wptr.lock();
			if (!obj)
			{
				continue;
			}
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

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) 
	{
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
			player.allowControl = false;
			canChangeScene = true;
		}
		else {
			camera.Set(prevMode);
			player.allowControl = true;
			canChangeScene = false;
		}
	}

	// Player controls
	if (player.allowControl) {

		// Movements
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

		// Actions
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::LMB)) {
			if (currentActiveNanobotAmmo < maxNanobotAmmo)
			{
				isNanobotFired = true;
			}
		}
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB)) {
		}
		if (MouseController::GetInstance()->IsButtonPressed(MouseController::MMB)) {
		}
		if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) > 0) {
		}
		if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) < 0) {
		}

		// ******************************************************************************************
		// ADDITIONAL Controls for SceneMedical
		// ******************************************************************************************
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SLASH))
		{
			if (isHelpOpen)
			{
				isHelpOpen = false;
				menuChange = true;
			}
			else
			{
				isHelpOpen = true;
				menuChange = true;
			}
		}
	}





	// ***************************************************************
	// Replay Mechanic
	// ***************************************************************
	if (currentState == MedicalGameState::RESULTS && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_BACKSPACE))
	{
		ClearResultsMenu();
		currentState = MedicalGameState::PLAYING;
		camera.Set(Cam::MODE::FIRST_PERSON);
		player.allowControl = true;
		isInResults = false;
	}





	// ******************************************************************************************************************************
	// ******************************************************************************************************************************
	// SCENE CHANGING IS DONE HERE
	// ******************************************************************************************************************************
	// ******************************************************************************************************************************
	if (canChangeScene && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_ENTER))
	{
		// however needed to change scene
		// ...
		// If you need the game data to save highscores, there is GetBestTimeForMedical() and GetGameGradeForMedical taking int and std::string respectively
		canChangeScene = false; // Once scene has changed already reset to false
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
		newObj->trl = vec3(-0.98f, -0.95f + i * 0.05f, 0);
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





// ******************************************************************
// Win/Lose Condition Related Management for SceneMedical
// ******************************************************************
void SceneMedical::HandleWinCondition()
{
	if (bestTimeTaken == 0 || totalTimeTaken < bestTimeTaken)
	{
		bestTimeTaken = totalTimeTaken;
	}

	// Use if needed for Aizzul's suggestion on average game across all integrated scenes
	if (bestTimeTaken <= 240.f)
	{
		gameGrade = "A";
	}
	else if (bestTimeTaken <= 300.f && bestTimeTaken > 240.f)
	{
		gameGrade = "B";
	}
	else if (bestTimeTaken <= 360.f && bestTimeTaken > 300.f)
	{
		gameGrade = "C";
	}
	else if (bestTimeTaken <= 540.f && bestTimeTaken > 360.f)
	{
		gameGrade = "D";
	}
	else if (bestTimeTaken > 540.f)
	{
		gameGrade = "F";
	}

	canChangeScene = true;
	ShowResultsMenu();
}





// ********************************************************************
// Gameplay Related Management For SceneMedical
// ********************************************************************
void SceneMedical::ChangeWave(int waveNumber)
{
	currentSpawningP = 0;
	currentSpawningAI = 0;

	waveTimeLeft = 180;

	bacteriaSpawnTimer = 0.0f;
	virusSpawnTimer = 0.0f;

	if (waveNumber == 1)
	{
		this->waveNumber = waveNumber;

		maxEntitiesP = 10;
		maxEntitiesAI = 5;
		
		remainingEntitiesP = 10;
		remainingEntitiesAI = 5;
	}
	else if (waveNumber == 2)
	{
		this->waveNumber = waveNumber;

		maxEntitiesP = 15;
		maxEntitiesAI = 10;
		
		remainingEntitiesP = 15;
		remainingEntitiesAI = 10;
	}
	else if (waveNumber == 3)
	{
		this->waveNumber = waveNumber;

		maxEntitiesP = 15;
		maxEntitiesAI = 15;

		remainingEntitiesP = 15;
		remainingEntitiesAI = 15;
	}
	else
	{
		waveNumber = 1; // Set to default if somehow fails and is not between 1 to 3
		std::cout << "Wave input may be incorrect, please check and retry again" << std::endl;
	}
}

void SceneMedical::ShowOverloadStack()
{
	for (int i = static_cast<int>(overloadStackUI.size()) - 1; i >= 0; --i)
	{
		auto& overloadUI = overloadStackUI[i];
		overloadUI.object->Destroy();
		overloadStackUI.erase(overloadStackUI.begin() + i);
	}

	if (overloadingStack <= 0)
	{
		return;
	}

	for (int i = 0; i < overloadingStack; i++)
	{
		GEOMETRY_TYPE mesh;

		if (i < 2)
			mesh = GAME_OVERLOADSTACK_G;
		else if (i < 4)
			mesh = GAME_OVERLOADSTACK_Y;
		else
			mesh = GAME_OVERLOADSTACK_R;

		screenRoot->NewChild(MeshObject::Create(mesh, 2));
		std::shared_ptr<RenderObject> obj = RenderObject::newObject;

		obj->relativeTrl = true;
		obj->trl = glm::vec3(0.5825f + i * 0.08375f, -0.9f, 0);
		obj->scl = glm::vec3(63.f, 30.f, 1);

		Overload ov;
		ov.object = obj;

		overloadStackUI.push_back(ov);
	}
}





// ****************************************************************
// Menu Management For SceneMedical
// ****************************************************************
void SceneMedical::ShowHelpMenu()
{
	for (int i = 0; i < 2; i++)
	{
		GEOMETRY_TYPE mesh;
		int layerIndex;

		if (i == 0)
		{
			mesh = GAME_UI_BASE;
			layerIndex = 0;
		}
		else
		{
			mesh = GAME_UI_PLATE;
			layerIndex = 1;
		}

		screenRoot->NewChild(MeshObject::Create(mesh, layerIndex));
		std::shared_ptr<RenderObject> help = RenderObject::newObject;

		help->relativeTrl = true;
		help->scl = glm::vec3(800.f - i * 10.f, 200.f - i * 10.f, 1);

		GameMenu gm;
		gm.object = help;

		menus.push_back(gm);
	}
	for (int i = 0; i < 6; i++)
	{
		std::string intendedText;

		if (i == 0)
		{
			intendedText = "LMB to shoot nanobots";
		}
		if (i == 1)
		{
			intendedText = "ALT to Pause   |   (Enter) while in pause to change scene";
		}
		if (i == 2)
		{
			intendedText = "(/) to re-open Help";
		}
		if (i == 4)
		{
			intendedText = "Objective: Defeat all waves of bacteria and viruses within the time";
		}
		if (i == 5)
		{
			intendedText = "Taking hits may cause you to overload and be set back a little...";
		}
		auto helpText = AddFlexText("Instruct", intendedText, glm::vec3(-0.4625f, 0.125f - i * 0.05f, 0), glm::vec3(30, 30, 1), FONT_CASCADIA_MONO);
		textObjects.push_back(helpText);
	}
}

void SceneMedical::ClearHelpMenu()
{
	for (int i = static_cast<int>(menus.size()) - 1; i >= 0; --i)
	{
		auto& helpUI = menus[i];
		helpUI.object->Destroy();
		menus.erase(menus.begin() + i);
	}
	for (auto& obj : textObjects)
    {
		if (obj)
		{
		   obj->Destroy();   // or RemoveFromParent()
		}
    }
    textObjects.clear();
}

void SceneMedical::ShowResultsMenu()
{
	for (int i = 0; i < 2; i++)
	{
		GEOMETRY_TYPE mesh;
		int layerIndex;

		if (i == 0)
		{
			mesh = GAME_UI_BASE;
			layerIndex = 0;
		}
		else
		{
			mesh = GAME_UI_PLATE;
			layerIndex = 1;
		}

		screenRoot->NewChild(MeshObject::Create(mesh, layerIndex));
		std::shared_ptr<RenderObject> help = RenderObject::newObject;

		help->relativeTrl = true;
		help->scl = glm::vec3(400.f - i * 10.f, 200.f - i * 10.f, 1);

		GameMenu gm;
		gm.object = help;

		menus.push_back(gm);
	}
	for (int i = 0; i < 7; i++)
	{
		std::string intendedText;

		if (i == 0)
		{
			intendedText = "Best Time: " + std::to_string(bestTimeTaken) + "s";
		}
		if (i == 1) // Use if needed for Aizzul's suggestion on average grade across all integrated scenes
		{
			intendedText = "Current Grade: " + gameGrade;
		}
		if (i == 3)
		{
			intendedText = "This Time: " + std::to_string(totalTimeTaken) + "s";
		}
		if (i == 5)
		{
			intendedText = "(<--) to retry";
		}
		if (i == 6)
		{
			intendedText = "(Enter) to go to next game";
		}

		auto helpText = AddFlexText("Results", intendedText, glm::vec3(-0.2f, 0.15f - i * 0.05f, 0), glm::vec3(30, 30, 1), FONT_CASCADIA_MONO);
		textObjects.push_back(helpText);
	}
}

void SceneMedical::ClearResultsMenu()
{
	for (int i = static_cast<int>(menus.size()) - 1; i >= 0; --i)
	{
		auto& resultsUI = menus[i];
		resultsUI.object->Destroy();
		menus.erase(menus.begin() + i);
	}
	for (auto& obj : textObjects)
	{
		if (obj)
		{
			obj->Destroy();   // or RemoveFromParent()
		}
	}
	textObjects.clear();
}





// **************************************************************
// Text Management For SceneMedical
// **************************************************************
std::shared_ptr<TextObject> SceneMedical::AddFlexText(const std::string& name, const std::string& text, vec3 pos, vec3 scl, GEOMETRY_TYPE font)
{
	screenRoot->NewChild(TextObject::Create(name, text, vec3(1, 1, 1), font, false, 99));

	auto obj = RObj::newObject;
	obj->relativeTrl = true;
	obj->trl = pos;
	obj->scl = scl;

	return std::dynamic_pointer_cast<TextObject>(obj);
}

void SceneMedical::InitSceneMedicalText(GEOMETRY_TYPE font) {
	auto& newObj = RObj::newObject;
	for (int i = 0; i < 10; i++) {
		screenRoot->NewChild(TextObject::Create("SceneMedical Text" + std::to_string(i), "", vec3(0, 1, 1), font, false, 99));
		newObj->relativeTrl = true;
		newObj->trl = vec3(-0.98f, 0.95f - i * 0.05f, 0);
		newObj->scl = vec3(30, 30, 1);
		sceneMedicalTextList.push_back(newObj);
	}
}

bool SceneMedical::AddSceneMedicalText(const std::string& text, int index) {

	if (index < 0) {
		for (auto& obj_weak : sceneMedicalTextList) {
			auto textObj = std::dynamic_pointer_cast<TextObject>(obj_weak.lock());

			if (textObj->text == "") {
				textObj->text = text;
				return true;
			}
		}
		return false;
	}

	index = Clamp(index, 0, 9);
	std::dynamic_pointer_cast<TextObject>(sceneMedicalTextList[index].lock())->text = text;

	return true;
}

void SceneMedical::ClearSceneMedicalText() {
	for (auto& obj_weak : sceneMedicalTextList)
		std::dynamic_pointer_cast<TextObject>(obj_weak.lock())->text = "";
}

int SceneMedical::GetBestTimeForMedical()
{
	return bestTimeTaken;
}

std::string SceneMedical::GetGameGradeForMedical()
{
	return gameGrade;
}