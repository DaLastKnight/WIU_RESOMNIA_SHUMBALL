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
#include "AudioManager.h"
#include "DataManager.h"

#include "Utils.h"

using App = Application;
using RObj = RenderObject;

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

}

void BaseScene::Update(double dt) {

	// refreshes and clears per frame
	RObj::newObject.reset();
	camera.VariableRefresh();
	player.VariableRefresh();

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
			lightProperties.spotDirection = -viewStack.Top() * glm::vec4(lightProperties.spotDirection, 0);// spotDirection_cameraspace
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

}

void BaseScene::Exit()
{
	// Cleanup VBO here
	for (auto& mesh : meshList)
		if (mesh)
			delete mesh;

	DataManager::GetInstance().SaveData();

	AudioManager::GetInstance().PauseMUS();
	AudioManager::GetInstance().UnloadAll();

	worldRoot.reset();
	viewRoot.reset();
	screenRoot.reset();

	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}

void BaseScene::HandleKeyPress()
{
	
}


/*********************************************************************************************************************************************************************************/
/************************************************************************************ helpers ************************************************************************************/
/*********************************************************************************************************************************************************************************/

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

