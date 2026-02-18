#ifndef BASE_SCENE_H
#define BASE_SCENE_H

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <memory>

#include "Scene.h"
#include "Mesh.h"
#include "FPCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "Atmosphere.h"

#include "RenderObject.h"


class BaseScene : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_GROUND,
		GEO_SKYBOX,
		GEO_LIGHT,
		GEO_GROUP,

		FONT_CASCADIA_MONO,

		// add more variables here
		UI_TEST,
		UI_TEST_2,

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

	float fontSpacing(GEOMETRY_TYPE font);

	// HandleInput functions
	void HandleKeyPress();
	
	// Geometry/Shader members
	Mesh* meshList[NUM_GEOMETRY];

	// uniforms for shader
	static constexpr int MAX_LIGHT = 12;
	void UpdateLightUniform(const std::shared_ptr<LightObject>& lightObj, LIGHT_UNIFORM_TYPE uniform = U_LIGHT_TOTAL);

	void UpdateAtmosphereUniform(ATMOSPHERE_UNIFORM_TYPE uniform = U_ATMOSPHERE_TOTAL);
	bool enabledAtmosphere = true;

	// Matrix Stack & projection members
	MatrixStack modelStack, viewStack, projectionStack;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	Atmosphere atmosphere;

	FPCamera camera;

	std::shared_ptr<RenderObject> worldRoot;
	std::shared_ptr<RenderObject> viewRoot;
	std::shared_ptr<RenderObject> screenRoot;

	glm::mat4 perspective;
	glm::mat4 ortho;

	void RenderMesh(GEOMETRY_TYPE object, bool enableLight);
	void renderObject(const std::shared_ptr<RenderObject> obj);

	// debug
	bool debug = false;
	// if passed in index, overwrite data in that specific debug text
	// returns success
	// does not work in Init()
	bool AddDebugText(const std::string& text, int index = -1); 

private:

	void clearDebugText();

	// Geometry/Shader members
	unsigned m_vertexArrayID;

	// uniforms for shader
	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	std::array<std::array<unsigned, U_LIGHT_TOTAL>, MAX_LIGHT> lightUniformLocations;
	std::array<unsigned, U_ATMOSPHERE_TOTAL> atmosphereUniformLocations;

	std::vector<std::weak_ptr<RenderObject>> debugTextList;
};

#endif