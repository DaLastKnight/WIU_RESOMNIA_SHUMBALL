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
#include "LoadTGA.h"
#include "MouseController.h"
#include "KeyboardController.h"
#include "DataManager.h"

#include "Utils.h"

using App = Application;

using glm::vec3;
using glm::mat4;
using std::string;


BaseScene::BaseScene()
{
}

BaseScene::~BaseScene()
{
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
		meshList[GEO_PLANE] = MeshBuilder::GenerateQuad("ground", vec3(1, 1, 1), 1000, 1000);
		meshList[GEO_PLANE]->textureID = LoadTGA("Image/color.tga");
		meshList[GEO_SKYBOX] = MeshBuilder::GenerateSkybox("skybox");
		meshList[GEO_SKYBOX]->textureID = LoadTGA("Image/skybox.tga");
		meshList[GEO_SKYBOX]->material.Set(vec3(1), vec3(0), vec3(0), 1);
	}

	// light init
	{
		light.reserve(12);
		unsigned lightIndex;

		light.emplace_back();
		lightIndex = 0;
		light[lightIndex].position = glm::vec3(0.f, 5.f, 0.f);
		light[lightIndex].color = glm::vec3(1, 1, 1);
		light[lightIndex].type = Light::LIGHT_POINT;
		light[lightIndex].power = 1;
		light[lightIndex].kC = 1.f;
		light[lightIndex].kL = 0.01f;
		light[lightIndex].kQ = 0.001f;

		UpdateLightUniform(light[lightIndex]);
	}

	// atmosphere init
	{
		atmosphere.Set(vec3(0.05f, 0.07f, 0.1f), 0.05f, 0.000001f, 2, 20);
		UpdateAtmosphereUniform();
	}


}

void BaseScene::Update(double dt)
{
	camera.VariableRefresh();
	HandleKeyPress();
	camera.Update(dt);
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
	for (unsigned i = 0; i < light.size(); i++) {
		auto& lightData = light[i];
		if (lightData.type == Light::LIGHT_DIRECTIONAL) {
			// use light pos as the dire of light
			glm::vec3 lightPos_local = lightData.position;
			lightData.position = viewStack.Top() * glm::vec4(lightData.position, 0); // lightDirection_cameraspace
			UpdateLightUniform(lightData, U_LIGHT_POSITION);
			lightData.position = lightPos_local;
		}
		else if (lightData.type == Light::LIGHT_SPOT) {
			glm::vec3 lightPos_local = lightData.position;
			lightData.position = viewStack.Top() * glm::vec4(lightData.position, 1); // lightPosition_cameraspace
			UpdateLightUniform(lightData, U_LIGHT_POSITION);
			lightData.position = lightPos_local;
			glm::vec3 spotDire_local = lightData.spotDirection;
			lightData.spotDirection = viewStack.Top() * glm::vec4(lightData.spotDirection, 0);// spotDirection_cameraspace
			UpdateLightUniform(lightData, U_LIGHT_SPOTDIRECTION);
			lightData.spotDirection = spotDire_local;
		}
		else {
			// Calculate the light position in camera space
			glm::vec3 lightPos_local = lightData.position;
			lightData.position = viewStack.Top() * glm::vec4(lightData.position, 1); // lightPosition_cameraspace
			UpdateLightUniform(lightData, U_LIGHT_POSITION);
			lightData.position = lightPos_local;
		}
	}

	// Define the projection matrix
	if (projType) // PERSPECTIVE
		projectionStack.LoadMatrix(glm::perspective(45.f, App::ASPECT_RATIO, 0.1f, 1000.f));
	else // ORTHOGRAPHICS
		projectionStack.LoadMatrix(glm::ortho(0.f, App::SCREEN_WIDTH, 0.f, App::SCREEN_HEIGHT, -1000.f, 1000.f));

	// Calculate the Model-View-Project matrix
	glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));

	modelStack.PushMatrix();
	RenderMesh(meshList[GEO_AXES], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Rotate(-90, 1, 0, 0);
	RenderMesh(meshList[GEO_PLANE], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	RenderMesh(meshList[GEO_SKYBOX], true);
	modelStack.PopMatrix();

}

void BaseScene::RenderMesh(Mesh* mesh, bool enableLight)
{
	//glDisable(GL_CULL_FACE);
	
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

void BaseScene::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
{
	glDisable(GL_DEPTH_TEST);
	glm::mat4 ortho = glm::ortho(0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f); // dimension of screen UI
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	
	viewStack.PushMatrix();
	viewStack.LoadIdentity(); // No need camera for ortho mode
	modelStack.PushMatrix();
	modelStack.LoadIdentity();

	modelStack.Translate(x, y, 0.f);
	modelStack.Scale(sizex, sizey, 1.f);

	RenderMesh(mesh, false); //UI should not have light
	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void BaseScene::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
{
	if (!mesh || mesh->textureID <= 0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHT_ENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);

	for (unsigned i = 0; i < text.length(); ++i)
	{
		glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(i * 1.0f, 0, 0));
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
		mesh->Render((unsigned)text[i] * 6, 6);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void BaseScene::RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y)
{
	if (!mesh || mesh->textureID <= 0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glm::mat4 ortho = glm::ortho(0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f);

	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();

	modelStack.PushMatrix();
	modelStack.LoadIdentity();

	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);

	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHT_ENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);

	for (unsigned i = 0; i < text.length(); ++i)
	{
		glm::mat4 characterSpacing = glm::translate(glm::mat4(1.f), glm::vec3(0.5f + i * 1.0f, 0.5f, 0));
		glm::mat4 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
		mesh->Render((unsigned)text[i] * 6, 6);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
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

	if (KeyboardController::GetInstance()->IsKeyPressed(VK_TAB))
	{
		// Change to black background
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
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

	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}

void BaseScene::UpdateLightUniform(const Light& lightData, LIGHT_UNIFORM_TYPE uniform) {

	if (light.empty())
		return;

	unsigned lightIndex = &lightData - &light[0];

	switch (uniform) {
	case U_LIGHT_TYPE: glUniform1i(lightUniformLocations[lightIndex][U_LIGHT_TYPE], lightData.type); break;
	case U_LIGHT_POSITION: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_POSITION], 1, glm::value_ptr(lightData.position)); break;
	case U_LIGHT_COLOR: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_COLOR], 1, glm::value_ptr(lightData.color)); break;
	case U_LIGHT_POWER: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_POWER], lightData.power); break;
	case U_LIGHT_KC: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KC], lightData.kC); break;
	case U_LIGHT_KL:glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KL], lightData.kL); break;
	case U_LIGHT_KQ: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KQ], lightData.kQ); break;
	case U_LIGHT_SPOTDIRECTION: glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_SPOTDIRECTION], 1, glm::value_ptr(lightData.spotDirection)); break;
	case U_LIGHT_COSCUTOFF: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSCUTOFF], cosf(glm::radians<float>(lightData.cosCutoff))); break;
	case U_LIGHT_COSINNER: glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSINNER], cosf(glm::radians<float>(lightData.cosInner))); break;
	
	default:
		glUniform1i(m_parameters[U_LIGHT_ENABLED], enabledLight);
		glUniform1i(m_parameters[U_LIGHT_NUMLIGHTS], light.size());
		glUniform1i(lightUniformLocations[lightIndex][U_LIGHT_TYPE], lightData.type);
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_POSITION], 1, glm::value_ptr(lightData.position));
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_COLOR], 1, glm::value_ptr(lightData.color));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_POWER], lightData.power);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KC], lightData.kC);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KL], lightData.kL);
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_KQ], lightData.kQ);
		glUniform3fv(lightUniformLocations[lightIndex][U_LIGHT_SPOTDIRECTION], 1, glm::value_ptr(lightData.spotDirection));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSCUTOFF], cosf(glm::radians<float>(lightData.cosCutoff)));
		glUniform1f(lightUniformLocations[lightIndex][U_LIGHT_COSINNER], cosf(glm::radians<float>(lightData.cosInner)));
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

void BaseScene::RenderDebugText() {
	
	
}
