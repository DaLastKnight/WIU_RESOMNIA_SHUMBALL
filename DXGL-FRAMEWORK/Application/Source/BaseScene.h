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

		U_LIGHT0_TYPE,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHT0_SPOTDIRECTION,
		U_LIGHT0_COSCUTOFF,
		U_LIGHT0_COSINNER,
		U_LIGHT0_EXPONENT,

		U_NUMLIGHTS,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_LIGHTENABLED,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,

		U_TOTAL,
	};

	BaseScene();
	~BaseScene();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:

	// Render helper functions
	void RenderMesh(Mesh* mesh, bool enableLight);
	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey);
	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	// HandleInput functions
	void HandleKeyPress();
	
	// Geometry/Shader members
	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	// Matrix Stack & projection members
	MatrixStack modelStack, viewStack, projectionStack;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	static constexpr int NUM_LIGHTS = 1;
	Light light[NUM_LIGHTS];
	bool enableLight;

	FPCamera camera;

};

#endif