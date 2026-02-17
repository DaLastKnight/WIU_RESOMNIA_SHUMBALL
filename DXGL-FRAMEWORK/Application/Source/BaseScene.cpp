#define _USE_MATH_DEFINES
#include <cmath>

#include "BaseScene.h"

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
#include "DataManager.h"

#include "Utils.h"

using App = Application;

using glm::vec3;
using glm::mat4;
using std::string;


/*****************************************************************************************************************************************************************************************/
/************************************************************************************ scene functions ************************************************************************************/
/*****************************************************************************************************************************************************************************************/

BaseScene::BaseScene() {
}

BaseScene::~BaseScene() {
}

void BaseScene::Init()
{
	// Set clear screen color
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	//Enable depth buffer and depth testing
	glEnable(GL_DEPTH_TEST);

	//Enable back face culling
	glEnable(GL_CULL_FACE);

	//Default to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Generate a default VAO for now
	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	// Load the shader programs
	m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Text_Atmospheric.fragmentshader");
	glUseProgram(m_programID);

	// init prespective
	perspective = glm::perspective(45.f, App::ASPECT_RATIO, 0.1f, 1000.f);
	ortho = glm::ortho(0.f, App::SCREEN_WIDTH, 0.f, App::SCREEN_HEIGHT, -1000.f, 1000.f);

	// Init uniforms
	{
	    // Get a handle for our "MVP" uniform
		m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
		m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
		m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
		m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
		m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
		m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
		m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");

		m_parameters[U_LIGHT_ENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
		m_parameters[U_LIGHT_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");

		m_parameters[U_ATMOSPHERE_ENABLED] = glGetUniformLocation(m_programID, "atmosphereEnabled");

		m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
		m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
		m_parameters[U_LIGHT_ENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
		m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
		m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");

		Mesh::SetMaterialLoc(m_parameters[U_MATERIAL_AMBIENT], m_parameters[U_MATERIAL_DIFFUSE], m_parameters[U_MATERIAL_SPECULAR], m_parameters[U_MATERIAL_SHININESS]);
	
		for (unsigned lightIndex = 0; lightIndex < MAX_LIGHT; lightIndex++) {
			string param;
			param.reserve(30);
			lightUniformLocations[lightIndex][U_LIGHT_TYPE] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].type").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_POSITION] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].position_cameraspace").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_COLOR] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].color").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_POWER] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].power").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_KC] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].kC").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_KL] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].kL").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_KQ] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].kQ").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_SPOTDIRECTION] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].spotDirection").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_COSCUTOFF] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].cosCutoff").c_str());
			lightUniformLocations[lightIndex][U_LIGHT_COSINNER] = glGetUniformLocation(m_programID, param.assign("lights[" + std::to_string(lightIndex) + "].cosInner").c_str());
		}

		atmosphereUniformLocations[U_ATMOSPHERE_COLOR] = glGetUniformLocation(m_programID, "atmosphere.color");
		atmosphereUniformLocations[U_ATMOSPHERE_FOG_DENSITY] = glGetUniformLocation(m_programID, "atmosphere.fogDensity");
		atmosphereUniformLocations[U_ATMOSPHERE_FOG_VISIBILITY] = glGetUniformLocation(m_programID, "atmosphere.fogVisibility");
		atmosphereUniformLocations[U_ATMOSPHERE_LIGHTEST_RANGE] = glGetUniformLocation(m_programID, "atmosphere.lightestRange");
		atmosphereUniformLocations[U_ATMOSPHERE_DENSEST_RANGE] = glGetUniformLocation(m_programID, "atmosphere.densestRange");
	}

	// atmosphere init
	{
		atmosphere.Set(vec3(0.05f, 0.07f, 0.1f), 0.05f, 0.000001f, 2, 20);
		UpdateAtmosphereUniform();
	}

	// Initialise camera properties
	camera.Init(glm::vec3(1, 2, -1), glm::vec3(-1, -1, 1));
	camera.Set(FPCamera::MODE::FREE);

	// Init VBO here
	{
		for (int i = 0; i < NUM_GEOMETRY; ++i)
		{
			meshList[i] = nullptr;
		}
		meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
		meshList[GEO_GROUND] = MeshBuilder::GenerateGround("ground", 1000, 5, TextureLoader::LoadTGA("color.tga"));
		meshList[GEO_SKYBOX] = MeshBuilder::GenerateSkybox("skybox", TextureLoader::LoadTGA("skybox.tga"));
		meshList[GEO_LIGHT] = MeshBuilder::GenerateSphere("light", vec3(1));
		meshList[GEO_GROUP] = MeshBuilder::GenerateSphere("group", vec3(1));
		

		meshList[FONT_CASCADIA_MONO] = MeshBuilder::GenerateText("cascadia mono font", 16, 16, fontSpacing(FONT_CASCADIA_MONO), TextureLoader::LoadTGA("Cascadia_Mono.tga"));
	}

	// init roots
	{
		worldRoot = std::make_shared<RenderObject>();
		worldRoot->renderType = RenderObject::WORLD;
		worldRoot->geometryType = GEO_GROUP;

		viewRoot = std::make_shared<RenderObject>();
		viewRoot->renderType = RenderObject::VIEW;
		viewRoot->geometryType = GEO_GROUP;

		screenRoot = std::make_shared<RenderObject>();
		screenRoot->renderType = RenderObject::SCREEN;
		screenRoot->geometryType = GEO_GROUP;

		LightObject::maxLight = MAX_LIGHT;
		LightObject::lightList.reserve(MAX_LIGHT);

		RenderObject::worldList.reserve(50);
		RenderObject::viewList.reserve(10);
		RenderObject::screenList.reserve(10);
	}

	// init default stats
	{
		RenderObject::setDefaultStat.Subscribe(GEO_AXES, [](const std::shared_ptr<RenderObject>& obj) {
			obj->material.Set(Material::NO_LIGHT);
			});
		RenderObject::setDefaultStat.Subscribe(GEO_GROUND, [](const std::shared_ptr<RenderObject>& obj) {
			obj->material.Set(vec3(0.1f), vec3(0.65f), vec3(0), 1);
			obj->offsetRot = vec3(-90, 0, 0);
			});
		RenderObject::setDefaultStat.Subscribe(GEO_SKYBOX, [](const std::shared_ptr<RenderObject>& obj) {
			obj->material.Set(Material::BRIGHT);
			});
		RenderObject::setDefaultStat.Subscribe(GEO_LIGHT, [](const std::shared_ptr<RenderObject>& obj) {
			obj->material.Set(Material::NEON);
			obj->offsetScl = vec3(0.05f);
			});
		RenderObject::setDefaultStat.Subscribe(GEO_GROUP, [](const std::shared_ptr<RenderObject>& obj) {
			});

		RenderObject::setDefaultStat.Subscribe(FONT_CASCADIA_MONO, [](const std::shared_ptr<RenderObject>& obj) {
			});
	}

	// world space init
	{
		worldRoot->NewChild(MeshObject::Create(GEO_AXES));

		worldRoot->NewChild(MeshObject::Create(GEO_GROUND));

		worldRoot->NewChild(MeshObject::Create(GEO_SKYBOX));

		// light init
		{
			std::shared_ptr<LightObject> newLightObj;

			worldRoot->NewChild(LightObject::Create(GEO_LIGHT));
			newLightObj = std::dynamic_pointer_cast<LightObject>(RenderObject::newObject);
			{
				newLightObj->trl = vec3(0, 20, 0);
				newLightObj->material.Set(Material::NEON);
				newLightObj->name = "demo light";
				auto& lightProperties = newLightObj->lightProperties;
				lightProperties.type = Light::LIGHT_POINT;
				lightProperties.color = vec3(1, 1, 1);
				lightProperties.power = 1;
				// 0 - 1 percentage of actual values applies
				lightProperties.kC = 1;
				lightProperties.kL = 0.001f;
				lightProperties.kQ = 0.001f;
				// spot light variables
				lightProperties.cosCutoff = 45.f;
				lightProperties.cosInner = 30.f;
				lightProperties.spotDirection = vec3(0.f, 1.f, 0.f);
				UpdateLightUniform(newLightObj);
			}
		}
	}

	// view space init
	{
		
	}

	// screen space init
	{

	}

	RenderObject::newObject.reset();
}

void BaseScene::Update(double dt) {
	if (dt > 0.1f) {
		dt = 0.1f;
	}

	auto& lightList = LightObject::lightList;
	auto& worldList = RenderObject::worldList;
	auto& viewList = RenderObject::viewList;
	auto& screenList = RenderObject::screenList;

	camera.VariableRefresh();
	HandleKeyPress();
	camera.Update(dt);



	// world render objects
	for (unsigned i = 0; i < worldList.size(); ) {
		if (worldList[i].expired()) {
			worldList.erase(worldList.begin() + i);
			continue;
		}

		auto obj = worldList[i].lock();

		switch (obj->geometryType) {
		case GEO_AXES:
		case GEO_LIGHT:
		case GEO_GROUP:
			obj->allowRender = debug;
			break;

		case GEO_SKYBOX:
			obj->trl = camera.GetFinalPosition();
			break;

		default:
			break;
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

			if (textObj->id == "fps") {
				static float timer = 0;
				static int frameCount = 0;
				const float fpsUpdateTime = 0.5f;
				timer += dt;
				frameCount++;
				if (timer >= fpsUpdateTime) {
					textObj->text = "fps: " + std::to_string(frameCount / timer);
					timer = 0;
					frameCount = 0;
				}
			}

			if (textObj->id.find("debug_") != std::string::npos) {
				textObj->text = "";
			}

			if (debug) {

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
		Light& lp = obj->lightProperties;


		obj->UpdateModel();

		if (obj->renderType == RenderObject::VIEW || obj->renderType == RenderObject::WORLD) {

			
			mat4 lightModel = obj->model;

			if (obj->renderType == RenderObject::VIEW) {
				mat4 inversedView = glm::inverse(viewStack.Top());
				lightModel = inversedView * lightModel; // original light model is in camera space
			}
			lp.position = getPosFromModel(lightModel);
			lp.spotDirection = rotateScaleWithModel(lightModel, obj->initialDire);
		}
		else {
			lp.power = 0;
		}

		UpdateLightUniform(obj);
		i++;
	}

}


void BaseScene::Render()
{
	// clear screens
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Define the model matrix and set it to identity
	modelStack.Clear();
	modelStack.LoadIdentity();

	// Define the view matrix and set it with camera position, target position and up direction
	glm::mat4 view = glm::lookAt(
		camera.GetFinalPosition(),
		camera.GetFinalTarget(),
		camera.GetFinalUp()
	);
	viewStack.LoadMatrix(view);

	// Calculate the light position in camera space
	auto& light = LightObject::lightList;
	for (unsigned i = 0; i < LightObject::lightList.size(); i++) {
		auto lightObj = light[i].lock();
		auto& lightProperties = lightObj->lightProperties;

		if (lightProperties.type == Light::LIGHT_DIRECTIONAL) {
			// use light pos as the dire of light
			glm::vec3 lightPos_local = lightProperties.position;
			lightProperties.position = viewStack.Top() * glm::vec4(lightProperties.position, 0); // lightDirection_cameraspace
			UpdateLightUniform(lightObj, U_LIGHT_POSITION);
			lightProperties.position = lightPos_local;
		}
		else if (lightProperties.type == Light::LIGHT_SPOT) {

			glm::vec3 lightPos_local = lightProperties.position;
			lightProperties.position = viewStack.Top() * glm::vec4(lightProperties.position, 1); // lightPosition_cameraspace
			UpdateLightUniform(lightObj, U_LIGHT_POSITION);
			lightProperties.position = lightPos_local;

			glm::vec3 spotDire_local = lightProperties.spotDirection;
			lightProperties.spotDirection = viewStack.Top() * glm::vec4(lightProperties.spotDirection, 0);// spotDirection_cameraspace
			UpdateLightUniform(lightObj, U_LIGHT_SPOTDIRECTION);
			lightProperties.spotDirection = spotDire_local;
		}
		else {
			// Calculate the light position in camera space
			glm::vec3 lightPos_local = lightProperties.position;
			lightProperties.position = viewStack.Top() * glm::vec4(lightProperties.position, 1); // lightPosition_cameraspace
			UpdateLightUniform(lightObj, U_LIGHT_POSITION);
			lightProperties.position = lightPos_local;
		}
	}

	// Define the projection matrix
	if (projType) // PERSPECTIVE
		projectionStack.LoadMatrix(perspective);
	else // ORTHOGRAPHICS
		projectionStack.LoadMatrix(ortho);

	// Calculate the Model-View-Project matrix
	glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

	
	// render scene
	struct ListInfo {
		std::shared_ptr<RenderObject> obj;
		mat4 model;
		float depth;
		ListInfo(std::shared_ptr<RenderObject> obj, mat4 model, float depth)
			: obj(obj), model(model), depth(depth) {}
	};

	std::vector<ListInfo> transparencyList;
	transparencyList.reserve(40);

	auto insert2TransparencyList = [&](const std::shared_ptr<RenderObject>& obj, const mat4& model, float depth) {

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
			renderObject(info.obj);
			modelStack.PopMatrix();
		}
		};

	auto renderObjectList = [&](const std::vector<std::weak_ptr<RenderObject>>& list) {
		for (auto& obj_wptr : list) {
			auto obj = obj_wptr.lock();
			modelStack.PushMatrix();
			modelStack.LoadMatrix(obj->model);

			if (obj->hasTransparency) {
				vec3 obj_worldPos = vec3(modelStack.Top()[3]);
				vec3 obj2CameraPos = camera.GetFinalPosition() - obj_worldPos;
				float depthSqr = obj2CameraPos.x * obj2CameraPos.x + obj2CameraPos.y * obj2CameraPos.y + obj2CameraPos.z * obj2CameraPos.z;
				insert2TransparencyList(obj, modelStack.Top(), depthSqr);
			}
			else
				renderObject(obj);

			modelStack.PopMatrix();
		}
		};

	renderObjectList(RenderObject::worldList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	renderTransparencyList();
	transparencyList.clear();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);


	viewStack.PushMatrix();
	viewStack.LoadIdentity();


	renderObjectList(RenderObject::viewList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	renderTransparencyList();
	transparencyList.clear();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);


	glDisable(GL_DEPTH_TEST);

	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);


	renderObjectList(RenderObject::screenList);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	renderTransparencyList();
	transparencyList.clear();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.Clear();

	glEnable(GL_DEPTH_TEST);

}

void BaseScene::Exit()
{
	// Cleanup VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if (meshList[i])
		{
			delete meshList[i];
		}
	}

	DataManager::GetInstance().SaveData();

	worldRoot.reset();
	viewRoot.reset();
	screenRoot.reset();

	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}


/*********************************************************************************************************************************************************************************/
/************************************************************************************ helpers ************************************************************************************/
/*********************************************************************************************************************************************************************************/

float BaseScene::fontSpacing(GEOMETRY_TYPE font) {
	switch (font) {
	case FONT_CASCADIA_MONO: return 0.375f;
	default: return 1;
	}
}

void BaseScene::HandleKeyPress()
{

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_LEFT_ALT)) {
		static auto prevMode = FPCamera::MODE::FREE;
		auto& cameraMode = camera.GetCurrentMode();

		if (cameraMode != FPCamera::MODE::PAUSE) {
			prevMode = cameraMode;
			camera.Set(FPCamera::MODE::PAUSE);
		}
		else {
			camera.Set(prevMode);
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(0x31))
	{
		// Key press to enable culling
		glEnable(GL_CULL_FACE);
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x32))
	{
		// Key press to disable culling
		glDisable(GL_CULL_FACE);
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x33))
	{
		// Key press to enable fill mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //default fill mode
	}
	if (KeyboardController::GetInstance()->IsKeyPressed(0x34))
	{
		// Key press to enable wireframe mode for the polygon
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe mode
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
		debug = !debug;
	}

}

void BaseScene::UpdateLightUniform(const std::shared_ptr<LightObject>& lightObj, LIGHT_UNIFORM_TYPE uniform) {
	const auto& lightProperties = lightObj->lightProperties;
	const auto& lightIndex = lightObj->lightIndex;

	switch (uniform) {
	case U_LIGHT_TYPE: glUniform1i(lightUniformLocations[lightIndex][U_LIGHT_TYPE], lightProperties.type); break;
	case U_LIGHT_POSITION: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_POSITION], 1, glm::value_ptr(lightProperties.position)); break;
	case U_LIGHT_COLOR: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_COLOR], 1, glm::value_ptr(lightProperties.color)); break;
	case U_LIGHT_POWER: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_POWER], lightProperties.power); break;
	case U_LIGHT_KC: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KC], lightProperties.kC); break;
	case U_LIGHT_KL:glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KL], lightProperties.kL); break;
	case U_LIGHT_KQ: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KQ], lightProperties.kQ); break;
	case U_LIGHT_SPOTDIRECTION: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_SPOTDIRECTION], 1, glm::value_ptr(lightProperties.spotDirection)); break;
	case U_LIGHT_COSCUTOFF: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSCUTOFF], cosf(glm::radians<float>(lightProperties.cosCutoff))); break;
	case U_LIGHT_COSINNER: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSINNER], cosf(glm::radians<float>(lightProperties.cosInner))); break;
	default:
		glUniform1i(m_parameters[U_LIGHT_ENABLED], enabledLight);
		glUniform1i(m_parameters[U_LIGHT_NUMLIGHTS], LightObject::lightList.size());
		glUniform1i(lightUniformLocations[lightIndex][U_LIGHT_TYPE], lightProperties.type);
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_POSITION], 1, glm::value_ptr(lightProperties.position));
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_COLOR], 1, glm::value_ptr(lightProperties.color));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_POWER], lightProperties.power);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KC], lightProperties.kC);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KL], lightProperties.kL);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KQ], lightProperties.kQ);
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_SPOTDIRECTION], 1, glm::value_ptr(lightProperties.spotDirection));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSCUTOFF], cosf(glm::radians<float>(lightProperties.cosCutoff)));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSINNER], cosf(glm::radians<float>(lightProperties.cosInner)));
	}
}


void BaseScene::UpdateAtmosphereUniform(ATMOSPHERE_UNIFORM_TYPE uniform) {
	switch (uniform) {
	case U_ATMOSPHERE_COLOR: glUniform3fv(atmosphereUniformLocations[U_ATMOSPHERE_COLOR], 1, glm::value_ptr(atmosphere.color)); break;
	case U_ATMOSPHERE_FOG_DENSITY: glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_FOG_DENSITY], atmosphere.fogDensity); break;
	case U_ATMOSPHERE_FOG_VISIBILITY: glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_FOG_VISIBILITY], atmosphere.fogVisibility); break;
	case U_ATMOSPHERE_LIGHTEST_RANGE: glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_LIGHTEST_RANGE], atmosphere.lightestRange); break;
	case U_ATMOSPHERE_DENSEST_RANGE: glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_DENSEST_RANGE], atmosphere.densestRange); break;
	default:
		glUniform1i(m_parameters[U_ATMOSPHERE_ENABLED], enabledAtmosphere);
		glUniform3fv(atmosphereUniformLocations[U_ATMOSPHERE_COLOR], 1, glm::value_ptr(atmosphere.color));
		glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_FOG_DENSITY], atmosphere.fogDensity);
		glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_FOG_VISIBILITY], atmosphere.fogVisibility);
		glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_LIGHTEST_RANGE], atmosphere.lightestRange);
		glUniform1f(atmosphereUniformLocations[U_ATMOSPHERE_DENSEST_RANGE], atmosphere.densestRange);
	}
}



void BaseScene::RenderMesh(GEOMETRY_TYPE object, bool enableLight) {

	Mesh* mesh = meshList[object];
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

void BaseScene::renderObject(const std::shared_ptr<RenderObject> obj) {

	if (!obj->allowRender)
		return;

	bool enableLight = true;
	if (obj->material.type == Material::NO_LIGHT || obj->renderType == RenderObject::SCREEN)
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
		float spacing = fontSpacing(static_cast<GEOMETRY_TYPE>(textObj->geometryType));
		if (textObj->centerText)
			modelStack.Translate(text.size() * spacing / -2.f + spacing / 2, 0, 0);

		for (unsigned i = 0; i < text.length(); ++i)
		{
			glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(i * spacing, 0, 0));
			glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
			glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

			mesh->Render((unsigned)text[i] * 6, 6);
		}

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

void BaseScene::RenderDebugText() {
	
	
}
