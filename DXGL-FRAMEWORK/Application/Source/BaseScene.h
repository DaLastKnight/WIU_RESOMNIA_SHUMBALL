#ifndef BASE_SCENE_H
#define BASE_SCENE_H

#include <iostream>
#include <vector>
#include <string>
#include "Scene.h"
#include "Mesh.h"
#include "FPCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "Atmosphere.h"

#include <array>

class BaseScene : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_SPHERE,
		GEO_CUBE,
		GEO_PLANE,
		GEO_TEXT,
		GEO_SKYBOX,
		
		NUM_GEOMETRY,
	};

	enum UNIFORM_TYPE
	{
		U_MVP = 0,

		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,

		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,

		U_LIGHT_ENABLED,
		U_LIGHT_NUMLIGHTS,

		U_ATMOSPHERE_ENABLED,

		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,

		U_TEXT_ENABLED,
		U_TEXT_COLOR,

		U_TOTAL,
	};

	enum LIGHT_UNIFORM_TYPE {

		U_LIGHT_TYPE = 0,
		U_LIGHT_POSITION,
		U_LIGHT_COLOR,
		U_LIGHT_POWER,
		U_LIGHT_KC,
		U_LIGHT_KL,
		U_LIGHT_KQ,
		U_LIGHT_SPOTDIRECTION,
		U_LIGHT_COSCUTOFF,
		U_LIGHT_COSINNER,

		U_LIGHT_TOTAL,
	};

	enum ATMOSPHERE_UNIFORM_TYPE {

		U_ATMOSPHERE_COLOR = 0,
		U_ATMOSPHERE_FOG_DENSITY,
		U_ATMOSPHERE_FOG_VISIBILITY,
		U_ATMOSPHERE_LIGHTEST_RANGE,
		U_ATMOSPHERE_DENSEST_RANGE,
		U_ATMOSPHERE_TARGET_POS_CAMERASPACE,

		U_ATMOSPHERE_TOTAL,
	};

	BaseScene();
	virtual~BaseScene(); // = 0;

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

protected:

	// Render helper functions
	void RenderMesh(Mesh* mesh, bool enableLight);
	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey);
	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	// HandleInput functions
	void HandleKeyPress();
	
	// Geometry/Shader members
	Mesh* meshList[NUM_GEOMETRY];

	// uniforms for shader
	static constexpr int MAX_LIGHT = 12;
	void UpdateLightUniform(const Light& lightObj, LIGHT_UNIFORM_TYPE uniform = U_LIGHT_TOTAL);
	bool enabledLight = true;

	void UpdateAtmosphereUniform(ATMOSPHERE_UNIFORM_TYPE uniform = U_ATMOSPHERE_TOTAL);
	bool enabledAtmosphere = true;

	// Matrix Stack & projection members
	MatrixStack modelStack, viewStack, projectionStack;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	std::vector<Light> light;
	Atmosphere atmosphere;

	FPCamera camera;


	// debug
	bool debug = false;
	void AddDebugText(const std::string& text) {
		debugTextList.emplace_back(text);
	}

	void RenderDebugText();

private:

	// Geometry/Shader members
	unsigned m_vertexArrayID;

	// uniforms for shader
	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	std::array<std::array<unsigned, U_LIGHT_TOTAL>, MAX_LIGHT> lightUniformLocations;
	std::array<unsigned, U_ATMOSPHERE_TOTAL> atmosphereUniformLocations;

	std::vector<std::string> debugTextList;
};

#endif