#define _USE_MATH_DEFINES
#include <cmath>

#include "SceneICA2.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "Application.h"
#include "MeshBuilder.h"
#include "LoadTGA.h"

SceneICA2::SceneICA2()
{
}

SceneICA2::~SceneICA2()
{
}

void SceneICA2::SetMouseCapture(bool enable)
{
	// Don't proceed if mouse is already in the correct state
	if (mouseCaptured == enable) return;
	
	// Store locked/unlocked mode of mouse
	mouseCaptured = enable;
	GLFWwindow* window = glfwGetCurrentContext();
	if (!window) return;

	// Change the mouse mode
	glfwSetInputMode(window, GLFW_CURSOR, enable ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	// Flush deltas so camera doesn't snap violently or do jitter behaviour
	MouseController::GetInstance()->GetMouseDeltaX();
	MouseController::GetInstance()->GetMouseDeltaY();
}

void SceneICA2::ResolveCollision(glm::vec3& box1Pos, const glm::vec2& box1HalfExtent, float box1Mass, glm::vec3& box2Pos, const glm::vec2& box2HalfExtent, float box2Mass)
{
	// This function assumes collision check is true and accurate
	AABB box1 = AABB::InitAABB(box1Pos, box1HalfExtent);
	AABB box2 = AABB::InitAABB(box2Pos, box2HalfExtent);

	// Computes all possible collisions, each with their own penetration distance
	float leftPD = box1.max.x - box2.min.x;
	float rightPD = box2.max.x - box1.min.x;
	float backPD = box1.max.y - box2.min.y;
	float frontPD = box2.max.y - box1.min.y;

	// Find the smallest penetration distance, using leftPD as a start
	float minPD = leftPD;
	glm::vec2 normal{ -1.f, 0.f };

	if (minPD > rightPD)
	{
		minPD = rightPD;
		normal = glm::vec2(1.f, 0.f);
	}

	if (minPD > backPD)
	{
		minPD = backPD;
		normal = glm::vec2(0.f, -1.f);
	}

	if (minPD > frontPD)
	{
		minPD = frontPD;
		normal = glm::vec2(0.f, 1.f);
	}

	// Heavier objects move slower compared to more lightweight objects
	float invMass1 = (box1Mass == 0.f) ? 0.f : (1.f / box1Mass);
	float invMass2 = (box2Mass == 0.f) ? 0.f : (1.f / box2Mass);
	float totalInvMass = invMass1 + invMass2;
	if (totalInvMass == 0.f) return; // Reject collision response if both objects are static

	totalInvMass = 1.f / totalInvMass;

	// Resolve penetration
	box1Pos += (totalInvMass * invMass1) * minPD * glm::vec3(normal.x, 0.f, normal.y);
	box2Pos -= (totalInvMass * invMass2) * minPD * glm::vec3(normal.x, 0.f, normal.y);
}

void SceneICA2::ApplyObjFacePlayer(const glm::vec3& objPos, const glm::vec3& playerPos, float offsetDeg)
{
	// Applies Translate and Rotate such that it's always facing the player
	glm::vec3 toPlayer = player.pos - objPos;
	toPlayer.y = 0.f;

	// Prevent rotation if target.pos and camera.pos share almost the exact same position
	if (glm::dot(toPlayer, toPlayer) > 0.000001f)
	{
		toPlayer = glm::normalize(toPlayer);

		float yawRad = atan2f(toPlayer.z, toPlayer.x);
		float yawDeg = glm::degrees(yawRad);

		modelStack.Translate(objPos.x, objPos.y, objPos.z);
		modelStack.Rotate(-yawDeg + offsetDeg, 0.f, 1.f, 0.f);
	}
	else
	{
		modelStack.Translate(objPos.x, objPos.y, objPos.z);
	}
}

void SceneICA2::StartDialogue(std::vector<std::string>& dialogue)
{
	inDialogue = true;
	dialogueIndex = 0;
	currentDialogue = &dialogue;
}

void SceneICA2::AdvanceDialogue()
{
	if (!currentDialogue) return;

	dialogueIndex++;

	if (dialogueIndex >= static_cast<int>(currentDialogue->size()))
		EndDialogue();
}

void SceneICA2::EndDialogue()
{
	inDialogue = false;
	dialogueIndex = 0;
	currentDialogue = nullptr;
	camera.isLockedOn = false;
}

void SceneICA2::IntroCutscene()
{
	// Plays the Intro cutscene at the beginning of the game
	speakerDialogue = "You";
	
	switch (introIndex)
	{
	case 0:
		inCutscene = true;
		StartDialogue(playerWokeUp);
		break;
	case 1:
		inCutscene = true;
		wantToLock = true;
		wantToTarget = glm::vec3(-10.f, -8.f, -50.2f);
		StartDialogue(playerLooking);
		break;
	case 2:
		inCutscene = true;
		wantToLock = true;
		wantToTarget = glm::vec3(5.f, -8.f, -50.f);
		StartDialogue(playerReady);
		break;
	case 3:
		inCutscene = false;
		break;
	default:
		break;
	}

	introIndex++;
}

void SceneICA2::MonkCutscene()
{
	// Plays the Monk cutscene once lantern puzzle is done
	if (!isLanternPuzzleSolved) return;
	
	switch (monkIndex)
	{
	case 0:
		speakerDialogue = "???";
		inCutscene = true;
		StartDialogue(monkFirst);
		break;
	case 1:
		speakerDialogue = "You";
		inCutscene = true;
		StartDialogue(playerMonkFirst);
		break;
	case 2:
		speakerDialogue = "???";
		inCutscene = true;
		StartDialogue(monkSecond);
		break;
	case 3:
		speakerDialogue = "You";
		inCutscene = true;
		wantToLock = true;
		wantToTarget = monk.pos + glm::vec3(0.f, 6.f, 0.f);
		StartDialogue(playerMonkSecond);
		break;
	case 4:
		speakerDialogue = "Evil Monk";
		inCutscene = true;
		StartDialogue(monkThird);
		break;
	case 5:
		for (int i = 2; i < NUM_LIGHTS; i++)
		{
			light[i].color = glm::vec3(0.85f, 0.25f, 0.20f);
			glUniform3fv(m_parameters[U_LIGHT0_COLOR + static_cast<UNIFORM_TYPE>(i * 11)], 1, &light[i].color.r);
		}

		fog.color = glm::vec3(0.01f, 0.f, 0.f);
		fog.density = 0.8f;

		glUniform3f(m_parameters[U_FOG_COLOR], fog.color.x, fog.color.y, fog.color.z);
		glUniform1f(m_parameters[U_FOG_DENSITY], fog.density);
		
		inCutscene = false;
		break;
	default:
		break;
	}

	monkIndex++;
}

void SceneICA2::RunChestMenu()
{
	// Menu displaying a combination puzzle where player needs to put in
	// the correct code to obtain the reward
	static bool isLeftUp = false;
	
	// Process Left Button
	if (!isLeftUp && MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT))
	{
		isLeftUp = true;

		double x = MouseController::GetInstance()->GetMousePositionX();
		double y = 900 - MouseController::GetInstance()->GetMousePositionY();

		// Submit code
		if (x >= 1060 && x <= 1140 && y >= 140 && y <= 260)
		{
			if (chestNum1 == 9 && chestNum2 == 4 && chestNum3 == 7)
			{
				chest.canInteract = false;
				isChestSolved = true;
				chestMenuActive = false;
				isInteracting = false;
				camera.isLockedOn = false;
			}
		}

		// Increment code numbers
		if (y >= 230 && y <= 275)
		{
			if (x >= 610 && x <= 690)
			{
				chestNum1++;
				if (chestNum1 % 10 == 0)
				{
					chestNum1 = 0;
				}
			}
			else if (x >= 760 && x <= 840)
			{
				chestNum2++;
				if (chestNum2 % 10 == 0)
				{
					chestNum2 = 0;
				}
			}
			else if (x >= 910 && x <= 990)
			{
				chestNum3++;
				if (chestNum3 % 10 == 0)
				{
					chestNum3 = 0;
				}
			}
		}
		else if (y >= 130 && y <= 175)
		{
			if (x >= 610 && x <= 690)
			{
				chestNum1--;
				if (chestNum1 < 0)
				{
					chestNum1 = 9;
				}
			}
			else if (x >= 760 && x <= 840)
			{
				chestNum2--;
				if (chestNum2 < 0)
				{
					chestNum2 = 9;
				}
			}
			else if (x >= 910 && x <= 990)
			{
				chestNum3--;
				if (chestNum3 < 0)
				{
					chestNum3 = 9;
				}
			}
		}
	}
	else if (isLeftUp && MouseController::GetInstance()->IsButtonUp(GLFW_MOUSE_BUTTON_LEFT))
	{
		isLeftUp = false;
	}
}

void SceneICA2::Init()
{
	// Set background color to dark blue
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

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
	m_programID = LoadShaders("Shader//Texture.vertexshader", "Shader//Text.fragmentshader");
	glUseProgram(m_programID);
	
	InitUniforms();

	// Initialise camera properties
	camera.Init(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	InitModels();

	glm::mat4 projection = glm::perspective(45.f, 16.f / 9.f, 0.1f, 1000.f);
	projectionStack.LoadMatrix(projection);

	InitGame();
	InitLight();
	InitPlayer();
	InitSkeleton();
	InitMonk();
	InitCrate();
	InitInteractables();
	InitEnvironment();
}

void SceneICA2::InitUniforms()
{
	// Init uniforms

	// Get a handle for our "MVP" uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");

	// Light uniforms
	m_parameters[U_LIGHT0_TYPE] = glGetUniformLocation(m_programID, "lights[0].type");
	m_parameters[U_LIGHT0_POSITION] = glGetUniformLocation(m_programID, "lights[0].position_cameraspace");
	m_parameters[U_LIGHT0_COLOR] = glGetUniformLocation(m_programID, "lights[0].color");
	m_parameters[U_LIGHT0_POWER] = glGetUniformLocation(m_programID, "lights[0].power");
	m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID, "lights[0].kC");
	m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID, "lights[0].kL");
	m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID, "lights[0].kQ");
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");

	m_parameters[U_LIGHT1_TYPE] = glGetUniformLocation(m_programID, "lights[1].type");
	m_parameters[U_LIGHT1_POSITION] = glGetUniformLocation(m_programID, "lights[1].position_cameraspace");
	m_parameters[U_LIGHT1_COLOR] = glGetUniformLocation(m_programID, "lights[1].color");
	m_parameters[U_LIGHT1_POWER] = glGetUniformLocation(m_programID, "lights[1].power");
	m_parameters[U_LIGHT1_KC] = glGetUniformLocation(m_programID, "lights[1].kC");
	m_parameters[U_LIGHT1_KL] = glGetUniformLocation(m_programID, "lights[1].kL");
	m_parameters[U_LIGHT1_KQ] = glGetUniformLocation(m_programID, "lights[1].kQ");
	m_parameters[U_LIGHT1_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[1].spotDirection");
	m_parameters[U_LIGHT1_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[1].cosCutoff");
	m_parameters[U_LIGHT1_COSINNER] = glGetUniformLocation(m_programID, "lights[1].cosInner");
	m_parameters[U_LIGHT1_EXPONENT] = glGetUniformLocation(m_programID, "lights[1].exponent");

	m_parameters[U_LIGHT2_TYPE] = glGetUniformLocation(m_programID, "lights[2].type");
	m_parameters[U_LIGHT2_POSITION] = glGetUniformLocation(m_programID, "lights[2].position_cameraspace");
	m_parameters[U_LIGHT2_COLOR] = glGetUniformLocation(m_programID, "lights[2].color");
	m_parameters[U_LIGHT2_POWER] = glGetUniformLocation(m_programID, "lights[2].power");
	m_parameters[U_LIGHT2_KC] = glGetUniformLocation(m_programID, "lights[2].kC");
	m_parameters[U_LIGHT2_KL] = glGetUniformLocation(m_programID, "lights[2].kL");
	m_parameters[U_LIGHT2_KQ] = glGetUniformLocation(m_programID, "lights[2].kQ");
	m_parameters[U_LIGHT2_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[2].spotDirection");
	m_parameters[U_LIGHT2_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[2].cosCutoff");
	m_parameters[U_LIGHT2_COSINNER] = glGetUniformLocation(m_programID, "lights[2].cosInner");
	m_parameters[U_LIGHT2_EXPONENT] = glGetUniformLocation(m_programID, "lights[2].exponent");

	m_parameters[U_LIGHT3_TYPE] = glGetUniformLocation(m_programID, "lights[3].type");
	m_parameters[U_LIGHT3_POSITION] = glGetUniformLocation(m_programID, "lights[3].position_cameraspace");
	m_parameters[U_LIGHT3_COLOR] = glGetUniformLocation(m_programID, "lights[3].color");
	m_parameters[U_LIGHT3_POWER] = glGetUniformLocation(m_programID, "lights[3].power");
	m_parameters[U_LIGHT3_KC] = glGetUniformLocation(m_programID, "lights[3].kC");
	m_parameters[U_LIGHT3_KL] = glGetUniformLocation(m_programID, "lights[3].kL");
	m_parameters[U_LIGHT3_KQ] = glGetUniformLocation(m_programID, "lights[3].kQ");
	m_parameters[U_LIGHT3_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[3].spotDirection");
	m_parameters[U_LIGHT3_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[3].cosCutoff");
	m_parameters[U_LIGHT3_COSINNER] = glGetUniformLocation(m_programID, "lights[3].cosInner");
	m_parameters[U_LIGHT3_EXPONENT] = glGetUniformLocation(m_programID, "lights[3].exponent");

	m_parameters[U_LIGHT4_TYPE] = glGetUniformLocation(m_programID, "lights[4].type");
	m_parameters[U_LIGHT4_POSITION] = glGetUniformLocation(m_programID, "lights[4].position_cameraspace");
	m_parameters[U_LIGHT4_COLOR] = glGetUniformLocation(m_programID, "lights[4].color");
	m_parameters[U_LIGHT4_POWER] = glGetUniformLocation(m_programID, "lights[4].power");
	m_parameters[U_LIGHT4_KC] = glGetUniformLocation(m_programID, "lights[4].kC");
	m_parameters[U_LIGHT4_KL] = glGetUniformLocation(m_programID, "lights[4].kL");
	m_parameters[U_LIGHT4_KQ] = glGetUniformLocation(m_programID, "lights[4].kQ");
	m_parameters[U_LIGHT4_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[4].spotDirection");
	m_parameters[U_LIGHT4_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[4].cosCutoff");
	m_parameters[U_LIGHT4_COSINNER] = glGetUniformLocation(m_programID, "lights[4].cosInner");
	m_parameters[U_LIGHT4_EXPONENT] = glGetUniformLocation(m_programID, "lights[4].exponent");

	m_parameters[U_LIGHT5_TYPE] = glGetUniformLocation(m_programID, "lights[5].type");
	m_parameters[U_LIGHT5_POSITION] = glGetUniformLocation(m_programID, "lights[5].position_cameraspace");
	m_parameters[U_LIGHT5_COLOR] = glGetUniformLocation(m_programID, "lights[5].color");
	m_parameters[U_LIGHT5_POWER] = glGetUniformLocation(m_programID, "lights[5].power");
	m_parameters[U_LIGHT5_KC] = glGetUniformLocation(m_programID, "lights[5].kC");
	m_parameters[U_LIGHT5_KL] = glGetUniformLocation(m_programID, "lights[5].kL");
	m_parameters[U_LIGHT5_KQ] = glGetUniformLocation(m_programID, "lights[5].kQ");
	m_parameters[U_LIGHT5_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[5].spotDirection");
	m_parameters[U_LIGHT5_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[5].cosCutoff");
	m_parameters[U_LIGHT5_COSINNER] = glGetUniformLocation(m_programID, "lights[5].cosInner");
	m_parameters[U_LIGHT5_EXPONENT] = glGetUniformLocation(m_programID, "lights[5].exponent");

	m_parameters[U_LIGHT6_TYPE] = glGetUniformLocation(m_programID, "lights[6].type");
	m_parameters[U_LIGHT6_POSITION] = glGetUniformLocation(m_programID, "lights[6].position_cameraspace");
	m_parameters[U_LIGHT6_COLOR] = glGetUniformLocation(m_programID, "lights[6].color");
	m_parameters[U_LIGHT6_POWER] = glGetUniformLocation(m_programID, "lights[6].power");
	m_parameters[U_LIGHT6_KC] = glGetUniformLocation(m_programID, "lights[6].kC");
	m_parameters[U_LIGHT6_KL] = glGetUniformLocation(m_programID, "lights[6].kL");
	m_parameters[U_LIGHT6_KQ] = glGetUniformLocation(m_programID, "lights[6].kQ");
	m_parameters[U_LIGHT6_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[6].spotDirection");
	m_parameters[U_LIGHT6_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[6].cosCutoff");
	m_parameters[U_LIGHT6_COSINNER] = glGetUniformLocation(m_programID, "lights[6].cosInner");
	m_parameters[U_LIGHT6_EXPONENT] = glGetUniformLocation(m_programID, "lights[6].exponent");

	// Fog uniforms
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");
	m_parameters[U_FOG_ENABLED] = glGetUniformLocation(m_programID, "fogEnabled");
	m_parameters[U_FOG_COLOR] = glGetUniformLocation(m_programID, "fogColor");
	m_parameters[U_FOG_DENSITY] = glGetUniformLocation(m_programID, "fogDensity");
	m_parameters[U_FOG_MINDIST] = glGetUniformLocation(m_programID, "fogMinDistance");
	m_parameters[U_FOG_MAXDIST] = glGetUniformLocation(m_programID, "fogMaxDistance");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");

	Mesh::SetMaterialLoc(m_parameters[U_MATERIAL_AMBIENT], m_parameters[U_MATERIAL_DIFFUSE], m_parameters[U_MATERIAL_SPECULAR], m_parameters[U_MATERIAL_SHININESS]);
}

void SceneICA2::InitModels()
{
	// Init VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = nullptr;
	}

	// Init mesh models
	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("Axes", 10000.f, 10000.f, 10000.f);
	meshList[GEO_SPHERE] = MeshBuilder::GenerateSphere("sphere", glm::vec3(1.f, 1.f, 1.f), 1.f, 16, 16);
	meshList[GEO_CUBE] = MeshBuilder::GenerateCube("cube", glm::vec3(1.f, 1.f, 1.f), 1.f);
	meshList[GEO_CUBE]->material.kAmbient = glm::vec3(0.01f, 0.01f, 0.01f);
	meshList[GEO_CUBE]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CUBE]->material.kSpecular = glm::vec3(0.01f, 0.01f, 0.01f);
	meshList[GEO_CUBE]->material.kShininess = 5.f;

	meshList[GEO_LEFT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_LEFT]->textureID = LoadTGA("Images//field-left.tga");
	meshList[GEO_RIGHT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_RIGHT]->textureID = LoadTGA("Images//field-right.tga");
	meshList[GEO_TOP] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_TOP]->textureID = LoadTGA("Images//field-top.tga");
	meshList[GEO_BOTTOM] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_BOTTOM]->textureID = LoadTGA("Images//field-bottom.tga");
	meshList[GEO_FRONT] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_FRONT]->textureID = LoadTGA("Images//field-front.tga");
	meshList[GEO_BACK] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 200.f, 5.f, 200.f);
	meshList[GEO_BACK]->textureID = LoadTGA("Images//field-back.tga");

	meshList[GEO_PLANE] = MeshBuilder::GenerateQuad("Plane", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_PLANE]->textureID = LoadTGA("Images//black.tga");
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16, 1.f);
	meshList[GEO_TEXT]->textureID = LoadTGA("Images//RenogareRegular.tga");

	meshList[GEO_STONEWALL] = MeshBuilder::GenerateOBJ("stonewall", "Models//stonewall.obj", false);
	meshList[GEO_STONEWALL]->textureID = LoadTGA("Images//stonewall.tga");
	meshList[GEO_STONEWALL]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_STONEWALL]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_STONEWALL]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_STONEWALL]->material.kShininess = 2.f;

	meshList[GEO_TREE] = MeshBuilder::GenerateOBJ("tree", "Models//tree.obj");
	meshList[GEO_TREE]->textureID = LoadTGA("Images//tree.tga");
	meshList[GEO_TREE]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_TREE]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_TREE]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_TREE]->material.kShininess = 2.f;

	meshList[GEO_TABLE] = MeshBuilder::GenerateOBJ("table", "Models//table.obj");
	meshList[GEO_TABLE]->textureID = LoadTGA("Images//table.tga");
	meshList[GEO_TABLE]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_TABLE]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_TABLE]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_TABLE]->material.kShininess = 2.f;

	meshList[GEO_LANTERN_UNLIT] = MeshBuilder::GenerateOBJ("lantern-unlit", "Models//lantern.obj");
	meshList[GEO_LANTERN_UNLIT]->textureID = LoadTGA("Images//lantern-unlit.tga");
	meshList[GEO_LANTERN_UNLIT]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_LANTERN_UNLIT]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_LANTERN_UNLIT]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_LANTERN_UNLIT]->material.kShininess = 2.f;

	meshList[GEO_LANTERN_LIT] = MeshBuilder::GenerateOBJ("lantern-lit", "Models//lantern.obj");
	meshList[GEO_LANTERN_LIT]->textureID = LoadTGA("Images//lantern-lit.tga");
	meshList[GEO_LANTERN_LIT]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_LANTERN_LIT]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_LANTERN_LIT]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_LANTERN_LIT]->material.kShininess = 2.f;

	meshList[GEO_DOOR] = MeshBuilder::GenerateOBJ("door", "Models//door.obj");
	meshList[GEO_DOOR]->textureID = LoadTGA("Images//door.tga");
	meshList[GEO_DOOR]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_DOOR]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_DOOR]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_DOOR]->material.kShininess = 2.f;

	meshList[GEO_SKELETON] = MeshBuilder::GenerateOBJMTL("skeleton", "Models//skeleton.obj", "Models//skeleton.mtl");
	meshList[GEO_SKELETON]->textureID = LoadTGA("Images//skeleton.tga");

	meshList[GEO_MONK] = MeshBuilder::GenerateOBJMTL("monk", "Models//monk.obj", "Models//monk.mtl");
	meshList[GEO_MONK]->textureID = LoadTGA("Images//monk.tga");

	meshList[GEO_FLASHLIGHT] = MeshBuilder::GenerateOBJ("flashlight", "Models//flashlight.obj", true);
	meshList[GEO_FLASHLIGHT]->textureID = LoadTGA("Images//flashlight.tga");
	meshList[GEO_FLASHLIGHT]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_FLASHLIGHT]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_FLASHLIGHT]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_FLASHLIGHT]->material.kShininess = 2.f;

	meshList[GEO_BOOK] = MeshBuilder::GenerateOBJ("book", "Models//book.obj");
	meshList[GEO_BOOK]->textureID = LoadTGA("Images//book.tga");
	meshList[GEO_BOOK]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_BOOK]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_BOOK]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_BOOK]->material.kShininess = 2.f;

	meshList[GEO_CRATE] = MeshBuilder::GenerateOBJMTL("crate", "Models//crate.obj", "Models//crate.mtl");
	meshList[GEO_CRATE]->textureID = LoadTGA("Images//crate.tga");

	meshList[GEO_CHEST] = MeshBuilder::GenerateOBJ("chest", "Models//chest.obj");
	meshList[GEO_CHEST]->textureID = LoadTGA("Images//chest.tga");
	meshList[GEO_CHEST]->material.kAmbient = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_CHEST]->material.kDiffuse = glm::vec3(1.f, 1.f, 1.f);
	meshList[GEO_CHEST]->material.kSpecular = glm::vec3(0.02f, 0.02f, 0.02f);
	meshList[GEO_CHEST]->material.kShininess = 2.f;

	meshList[GEO_2D_INTERACT] = MeshBuilder::GenerateQuad("interact-icon", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_INTERACT]->textureID = LoadTGA("Images//interact-icon.tga");

	meshList[GEO_2D_CHAT] = MeshBuilder::GenerateQuad("chat-icon", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_CHAT]->textureID = LoadTGA("Images//chat-icon.tga");

	meshList[GEO_2D_MAP] = MeshBuilder::GenerateQuad("map-icon", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_MAP]->textureID = LoadTGA("Images//map-icon.tga");

	meshList[GEO_2D_PAGE] = MeshBuilder::GenerateQuad("page", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_PAGE]->textureID = LoadTGA("Images//page.tga");

	meshList[GEO_2D_GOOD] = MeshBuilder::GenerateQuad("good-ending", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_GOOD]->textureID = LoadTGA("Images//good-ending.tga");

	meshList[GEO_2D_BAD] = MeshBuilder::GenerateQuad("bad-ending", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_BAD]->textureID = LoadTGA("Images//bad-ending.tga");

	meshList[GEO_2D_KEYLOCK] = MeshBuilder::GenerateQuad("keylock", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_KEYLOCK]->textureID = LoadTGA("Images//keylock.tga");

	meshList[GEO_2D_RIGHTARROW] = MeshBuilder::GenerateQuad("right-arrow", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_RIGHTARROW]->textureID = LoadTGA("Images//right-arrow.tga");

	meshList[GEO_2D_UPARROW] = MeshBuilder::GenerateQuad("up-arrow", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_UPARROW]->textureID = LoadTGA("Images//up-arrow.tga");

	meshList[GEO_2D_DOWNARROW] = MeshBuilder::GenerateQuad("down-arrow", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_DOWNARROW]->textureID = LoadTGA("Images//down-arrow.tga");

	meshList[GEO_2D_MOON] = MeshBuilder::GenerateQuad("moon", glm::vec3(1.f, 1.f, 1.f), 1.f, 1.f, 1.f);
	meshList[GEO_2D_MOON]->textureID = LoadTGA("Images//moon.tga");
}

void SceneICA2::InitGame()
{
	// Init game variables
	elapsedTime = 0;
	worldUIBob = 0.f;
	fps = 0;
	objectiveState = 0;
	objectiveText = "Find a light source";

	inCutscene = false;
	introIndex = 0;
	monkIndex = 0;

	inDialogue = false;
	dialogueIndex = 0;
	currentDialogue = nullptr;

	goodEndingActive = false;
	badEndingActive = false;
}

void SceneICA2::InitLight()
{
	// Init lights and fog
	// 7 Lights:
	// light[0]: Directional light from moon
	// light[1]: Spotlight from flashlight
	// light[2] - light[6]: Point light from lanterns
	
	glUniform1i(m_parameters[U_NUMLIGHTS], NUM_LIGHTS);

	light[0].position = glm::vec3(0.f, 50.f, 0.f);
	light[0].color = glm::vec3(1, 1, 1);
	light[0].type = Light::LIGHT_DIRECTIONAL;
	light[0].power = 0.4f;
	light[0].kC = 1.f;
	light[0].kL = 0.01f;
	light[0].kQ = 0.001f;
	light[0].cosCutoff = 45.f;
	light[0].cosInner = 30.f;
	light[0].exponent = 3.f;
	light[0].spotDirection = glm::vec3(0.f, 1.f, 0.f);

	light[1].position = glm::vec3(8.f, 2.f, 8.f);
	light[1].color = glm::vec3(1.f, 1.f, 1.f);
	light[1].type = Light::LIGHT_SPOT;
	light[1].power = 0.05f;
	light[1].kC = 1.f;
	light[1].kL = 0.1f;
	light[1].kQ = 0.001f;
	light[1].cosCutoff = 45.f;
	light[1].cosInner = 30.f;
	light[1].exponent = 1.f;
	light[1].spotDirection = glm::vec3(0.f, 1.f, 0.f);

	light[2].position = glm::vec3(15.f, -1.2f, 47.3f);
	light[3].position = glm::vec3(-15.f, -1.2f, 47.3f);
	light[4].position = glm::vec3(-47.3f, -1.2f, 0.f);
	light[5].position = glm::vec3(46.8f, -1.2f, 0.f);
	light[6].position = glm::vec3(0.f, -1.2f, -47.3f);

	for (int i = 2; i < NUM_LIGHTS; i++)
	{
		light[i].color = glm::vec3(1.f, 0.65f, 0.25f);
		light[i].type = Light::LIGHT_POINT;
		light[i].power = 2.f;
		light[i].kC = 1.f;
		light[i].kL = 0.1f;
		light[i].kQ = 0.001f;
		light[i].cosCutoff = 45.f;
		light[i].cosInner = 30.f;
		light[i].exponent = 1.f;
		light[i].spotDirection = glm::vec3(0.f, 1.f, 0.f);
	}

	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &light[0].color.r);
	glUniform1i(m_parameters[U_LIGHT0_TYPE], light[0].type);
	glUniform1f(m_parameters[U_LIGHT0_POWER], light[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], light[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], light[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], light[0].kQ);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], cosf(glm::radians<float>(light[0].cosCutoff)));
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], cosf(glm::radians<float>(light[0].cosInner)));
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], light[0].exponent);

	glUniform3fv(m_parameters[U_LIGHT1_COLOR], 1, glm::value_ptr(light[1].color));
	glUniform1i(m_parameters[U_LIGHT1_TYPE], light[1].type);
	glUniform1f(m_parameters[U_LIGHT1_POWER], light[1].power);
	glUniform1f(m_parameters[U_LIGHT1_KC], light[1].kC);
	glUniform1f(m_parameters[U_LIGHT1_KL], light[1].kL);
	glUniform1f(m_parameters[U_LIGHT1_KQ], light[1].kQ);
	glUniform1f(m_parameters[U_LIGHT1_COSCUTOFF], cosf(glm::radians<float>(light[1].cosCutoff)));
	glUniform1f(m_parameters[U_LIGHT1_COSINNER], cosf(glm::radians<float>(light[1].cosInner)));
	glUniform1f(m_parameters[U_LIGHT1_EXPONENT], light[1].exponent);

	for (int i = 2; i < NUM_LIGHTS; i++)
	{
		glUniform3fv(m_parameters[U_LIGHT0_COLOR + static_cast<UNIFORM_TYPE>(i * 11)], 1, &light[i].color.r);
		glUniform1i(m_parameters[U_LIGHT0_TYPE + static_cast<UNIFORM_TYPE>(i * 11)], light[i].type);
		glUniform1f(m_parameters[U_LIGHT0_POWER + static_cast<UNIFORM_TYPE>(i * 11)], light[i].power);
		glUniform1f(m_parameters[U_LIGHT0_KC + static_cast<UNIFORM_TYPE>(i * 11)], light[i].kC);
		glUniform1f(m_parameters[U_LIGHT0_KL + static_cast<UNIFORM_TYPE>(i * 11)], light[i].kL);
		glUniform1f(m_parameters[U_LIGHT0_KQ + static_cast<UNIFORM_TYPE>(i * 11)], light[i].kQ);
		glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF + static_cast<UNIFORM_TYPE>(i * 11)], cosf(glm::radians<float>(light[i].cosCutoff)));
		glUniform1f(m_parameters[U_LIGHT0_COSINNER + static_cast<UNIFORM_TYPE>(i * 11)], cosf(glm::radians<float>(light[i].cosInner)));
		glUniform1f(m_parameters[U_LIGHT0_EXPONENT + static_cast<UNIFORM_TYPE>(i * 11)], light[i].exponent);
	}

	enableLight = true;

	glUniform1i(m_parameters[U_FOG_ENABLED], 1);
	glUniform3f(m_parameters[U_FOG_COLOR], fog.color.x, fog.color.y, fog.color.z);
	glUniform1f(m_parameters[U_FOG_DENSITY], fog.density);
	glUniform1f(m_parameters[U_FOG_MINDIST], fog.minDist);
	glUniform1f(m_parameters[U_FOG_MAXDIST], fog.maxDist);
}

void SceneICA2::InitPlayer()
{
	// Init player
	wantToLock = false;
	equippedFlashlight = false;

	playerWokeUp = { "Ughh... where am I?" };
	playerLooking = { "And why is it so dark?" };
	playerReady = { "Some light would be great..." };
	
	playerDoorLocked = { "Doors are shut in tight...",
		"Who would have thought?" };

	playerMonkFirst = { "Who said that?!" };
	playerMonkSecond = { "What in the worl-" };
}

void SceneICA2::InitSkeleton()
{
	// Init skeleton
	skeleton.pos = skeleton.interaction.pos = glm::vec3(20.f, -5.f, -40.f);
	skeleton.halfExtent = glm::vec2(1.5f, 1.5f);
	isSkeletonSolved = false;
	skeleton.interaction.interactRadius = 10.f;

	skeletonEncounter = { "Well, lookie here, fresh meat!",
		"Looking for a way out? Hmm...",
		"Let me cut a deal with ya.",
		"Push those crates back there...",
		"and I'll give you a code!",
		"Well, better get to work then!" };

	skeletonEncounterRepeat = { "Well? What are you waiting for?" };

	skeletonComplete = { "You're done? Fantastic!",
		"Here's the code to the chest.", "It's 947. Now shoo." };

	skeletonCompleteRepeat = { "Nothing better to do?",
		"Huh? Already forgot the code?",
		"Sigh... it's 947" };
}

void SceneICA2::InitMonk()
{
	// Init monk
	monk.pos = glm::vec3(20.f, -6.f, 60.f);
	monk.halfExtent = glm::vec2(1.f, 1.f);
	monk.mass = 1.f;

	monkFirst = { "Was it worth the effort?",
		"Even though there's no escape?" };

	monkSecond = { "Turn around." };
	monkThird = { "Your time has come...",
		"NO ONE WILL DEFY DEATH ITSELF!" };
}

void SceneICA2::InitCrate()
{
	// Init crates
	crates[0].pos = glm::vec3(-27.f, -6.5f, 36.f);
	crates[0].halfExtent = glm::vec2(1.5f, 1.5f);
	crates[0].crateNum = 0;
	crates[0].mass = 0.f;

	crates[1].pos = glm::vec3(-35.f, -6.5f, -24.f);
	crates[1].halfExtent = glm::vec2(1.5f, 1.5f);
	crates[1].crateNum = 1;
	crates[1].mass = 0.f;

	crates[2].pos = glm::vec3(25.f, -6.5f, 36.f);
	crates[2].halfExtent = glm::vec2(1.5f, 1.5f);
	crates[2].crateNum = 2;
	crates[2].mass = 0.f;

	crates[3].pos = glm::vec3(2.f, -6.5f, -36.f);
	crates[3].halfExtent = glm::vec2(1.5f, 1.5f);
	crates[3].crateNum = 3;
	crates[3].mass = 0.f;

	crateTargetPos[0] = glm::vec3(40.f, -6.5f, -15.f);
	crateTargetPos[1] = glm::vec3(40.f, -6.5f, -5.f);
	crateTargetPos[2] = glm::vec3(40.f, -6.5f, 5.f);
	crateTargetPos[3] = glm::vec3(40.f, -6.5f, 15.f);
	crateTargetHalfExtent = glm::vec2(0.75f, 0.75f);

	for (int i = 0; i < 4; i++)
	{
		isCrateShifted[i] = false;
	}
}

void SceneICA2::InitInteractables()
{
	// Init interactive members
	isInteracting = false;

	// Init doors
	hasOpenedDoor = false;

	leftDoor.pos = glm::vec3(3.9f, -2.8f, 50.2f);
	leftDoor.interactRadius = 8.f;
	leftDoor.canInteract = true;
	leftDoorAngleDeg = 0.f;

	rightDoor.pos = glm::vec3(1.25f, -2.8f, 50.f);
	rightDoor.interactRadius = 8.f;
	rightDoor.canInteract = true;
	rightDoorAngleDeg = 180.f;

	// Init flashlight
	flashlight.pos = glm::vec3(20.f, -1.9f, 20.f);
	flashlight.canInteract = true;
	flashlight.interactRadius = 10.f;

	// Init chest
	chest.pos = glm::vec3(-20.f, -2.f, 20.f);
	chest.canInteract = true;
	chest.interactRadius = 10.f;
	chestNum1 = 0;
	chestNum2 = 0;
	chestNum3 = 0;

	// Init book
	book.pos = glm::vec3(-17.5f, -2.f, 20.f);
	book.canInteract = false;
	book.interactRadius = 10.f;
	bookMenuActive = false;
	hasReadBookOnce = false;

	// Init lanterns
	lanternOrderIndex = 0;
	isLanternPuzzleSolved = false;

	lanterns[0].pos = glm::vec3(15.f, -2.f, 47.5f);
	lanternAngleDeg[0] = 180.f;

	lanterns[1].pos = glm::vec3(-15.f, -2.f, 47.5f);
	lanternAngleDeg[1] = 180.f;

	lanterns[2].pos = glm::vec3(-47.5f, -2.f, 0.f);
	lanternAngleDeg[2] = 90.f;

	lanterns[3].pos = glm::vec3(47.f, -2.f, 0.f);
	lanternAngleDeg[3] = 270.f;

	lanterns[4].pos = glm::vec3(0.f, -2.f, -47.5f);
	lanternAngleDeg[4] = 0.f;

	for (int i = 0; i < 5; i++)
	{
		lanterns[i].canInteract = true;
		lanterns[i].interactRadius = 10.f;
		isLanternOn[i] = false;
		lanternOrder[i] = -1;
	}
}

void SceneICA2::InitEnvironment()
{
	// Init stone walls
	stonewalls.emplace_back(Wall::InitWall(glm::vec3(50.f, -5.f, 0.f), glm::vec2(2.f, 50.f)));
	stonewalls.emplace_back(Wall::InitWall(glm::vec3(-50.f, -5.f, 0.f), glm::vec2(2.f, 50.f)));
	stonewalls.emplace_back(Wall::InitWall(glm::vec3(0.f, -5.f, 50.f), glm::vec2(50.f, 2.f)));
	stonewalls.emplace_back(Wall::InitWall(glm::vec3(0.f, -5.f, -50.f), glm::vec2(50.f, 2.f)));

	// Init tables
	tables.emplace_back(Wall::InitWall(glm::vec3(-20.f, -5.f, -20.f), glm::vec2(4.5f, 7.4f)));
	tables.emplace_back(Wall::InitWall(glm::vec3(-20.f, -5.f, 20.f), glm::vec2(4.5f, 7.4f)));
	tables.emplace_back(Wall::InitWall(glm::vec3(20.f, -5.f, -20.f), glm::vec2(4.5f, 7.4f)));
	tables.emplace_back(Wall::InitWall(glm::vec3(20.f, -5.f, 20.f), glm::vec2(4.5f, 7.4f)));

	// Init trees
	trees.emplace_back(Wall::InitWall(glm::vec3(-35.f, -5.f, 0.f), glm::vec2(4.f, 4.f)));
	trees.emplace_back(Wall::InitWall(glm::vec3(25.f, -5.f, -40.f), glm::vec2(4.f, 4.f)));
	trees.emplace_back(Wall::InitWall(glm::vec3(-38.f, -5.f, -38.f), glm::vec2(4.f, 4.f)));
	trees.emplace_back(Wall::InitWall(glm::vec3(-34.f, -5.f, 39.f), glm::vec2(4.f, 4.f)));
	trees.emplace_back(Wall::InitWall(glm::vec3(40.f, -5.f, 40.f), glm::vec2(4.f, 4.f)));
}

void SceneICA2::Update(double dt)
{
	// Straight up ends game if any ending is achieved
	if (goodEndingActive || badEndingActive) return;
	
	// Determines whether cursor is locked and hidden or unlocked and free
	bool shouldCapture = !(inDialogue || isInteracting);
	camera.shouldCapture = shouldCapture;
	SetMouseCapture(shouldCapture);
	
	// Resets the locked camera mode so scene members don't fight over it
	wantToLock = false;
	wantToTarget = glm::vec3(0.f);
	
	UpdateGame(dt);
	UpdateLight(dt);
	UpdatePlayer(dt);
	UpdatePOV(dt);

	// Doesn't continue other game interactions while in dialogue or interaction (with menus)
	if (inDialogue)
	{
		// return; prevents player movement and other interactions during dialogue
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_SPACE))
		{
			AdvanceDialogue();
		}
		return;
	}

	
	if (isInteracting)
	{
		// return; prevents player movement and other interactions during dialogue
		if (chestMenuActive)
		{
			RunChestMenu();
		}
		
		if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E))
		{
			if (chestMenuActive) chestMenuActive = false;
			if (bookMenuActive) bookMenuActive = false;
			
			isInteracting = false;
			camera.isLockedOn = false;
		}
		return; // blocks UpdateSkeleton/UpdateItems fighting each other
	}

	IntroCutscene(); // Plays at the very beginning
	MonkCutscene(); // Plays after hitting a certain condition
	UpdateCrate(dt);
	UpdateInteractables(dt);
	UpdateMonk(dt);
	UpdateSkeleton(dt);
	
	HandleKeyPress();
	HandleMouseInput();

	// Determines locked camera mode
	camera.isLockedOn = wantToLock;
	if (wantToLock) camera.target = wantToTarget;
}

void SceneICA2::UpdateGame(double dt)
{
	worldUIBob = 0.2f * sinf(static_cast<float>(elapsedTime) * 3.f);
	elapsedTime += dt;
	float temp = 1.f / dt;
	fps = glm::round(temp * 100.f / 100.f);

	// Condition to hit another stage determined in other functions
	switch (objectiveState)
	{
	case 0:
		objectiveText = "Find a light source";
		break;
	case 1:
		objectiveText = "Find a way out";
		break;
	case 2:
		objectiveText = "Relocate the crates";
		break;
	case 3:
		objectiveText = "Use the code somewhere";
		break;
	case 4:
		objectiveText = "Light up the lanterns";
		break;
	case 5:
		objectiveText = "ESCAPE";
		break;
	default:
		break;
	}
}

void SceneICA2::UpdateLight(double dt)
{
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		// For lights in lanterns only
		if (i >= 2)
		{
			// Turn on if player has interacted with them/correct order
			if (isLanternOn[i - 2])
			{
				light[i].power = 2.f;
			}
			else
			{
				light[i].power = 0.01f;
			}
		}
		
		glUniform1f(m_parameters[U_LIGHT0_POWER + static_cast<UNIFORM_TYPE>(i * 11)], light[i].power);

		if (light[i].type == Light::LIGHT_DIRECTIONAL)
		{
			glm::vec3 lightDir(light[i].position.x, light[i].position.y, light[i].position.z);
			glm::vec3 lightDirection_cameraspace = viewStack.Top() * glm::vec4(lightDir, 0);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION + static_cast<UNIFORM_TYPE>(i * 11)], 1, glm::value_ptr(lightDirection_cameraspace));
		}
		else if (light[i].type == Light::LIGHT_SPOT)
		{
			glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[i].position, 1);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION + static_cast<UNIFORM_TYPE>(i * 11)], 1, glm::value_ptr(lightPosition_cameraspace));
			glm::vec3 spotDirection_cameraspace = viewStack.Top() * glm::vec4(light[i].spotDirection, 0);
			glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION + static_cast<UNIFORM_TYPE>(i * 11)], 1, glm::value_ptr(spotDirection_cameraspace));
		}
		else {
			// Calculate the light position in camera space
			glm::vec3 lightPosition_cameraspace = viewStack.Top() * glm::vec4(light[i].position, 1);
			glUniform3fv(m_parameters[U_LIGHT0_POSITION + static_cast<UNIFORM_TYPE>(i * 11)], 1, glm::value_ptr(lightPosition_cameraspace));
		}
	}
}

void SceneICA2::UpdatePOV(double dt)
{
	// View bobbing
	{
		player.bobAmp = (player.isMoving) ? 0.1f : 0.05f;
		player.bobFreq = (player.isMoving) ? 1.6f : 0.4f;
		float target = 1.f;
		player.bobAmount += (target - player.bobAmount) * (1.f - expf(-player.bobSmooth * dt));
		player.bobTimer += dt;

		// in case I want more bob speed for running instead of walking
		float speed = 1.f;
		float freq = player.bobFreq * speed;
		float amp = player.bobAmp * speed;

		player.bobOffsetY = sinf(player.bobTimer * freq * 2.f * M_PI) * amp * player.bobAmount;
	}

	camera.position = player.pos + glm::vec3(0.f, player.bobOffsetY, 0.f);

	// Updates camera and flashlight
	{
		camera.Update(dt);
		// Relative position of player's flashlight in respect to camera
		flashlightPos = camera.position + (camera.right * 1.f) + (camera.up * -0.3f) + (camera.front * 1.5f);

		// Adds polished adjustment to flashlight's position
		glm::vec3 cameraRightOffset = glm::normalize(glm::cross(camera.front, camera.up));
		glm::vec3 cameraUpOffset = glm::normalize(camera.up);
		// Similar to flashlightPos: light offset relative to flashlightPos this time around
		glm::vec3 offset = (cameraRightOffset * 0.15f) + (cameraUpOffset * -0.06f) + (camera.front * 0.6f);
		// Apply adjustment to light's position relative to flashlight's position
		light[1].position = flashlightPos + offset;
		light[1].spotDirection = glm::normalize(-camera.front);
	}
}

void SceneICA2::UpdatePlayer(double dt)
{
	if (!inDialogue && !isInteracting)
	{
		// Player movement
		{
			glm::vec3 worldUp{ 0.f, 1.f, 0.f };

			// Computes player's forward and right vectors in respect
			// to camera's front vector
			player.forward = glm::vec3(camera.front.x, 0.f, camera.front.z);

			// Prevent zero vector normalisation
			if (glm::length(player.forward) < 0.01f)
			{
				player.forward = glm::vec3(0.f, 0.f, -1.f);
			}
			else
			{
				player.forward = glm::normalize(player.forward);
			}

			player.right = glm::normalize(glm::cross(player.forward, worldUp));

			glm::vec3 moveDir{ 0.f, 0.f, 0.f };

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W))
			{
				moveDir += player.forward;
			}

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S))
			{
				moveDir -= player.forward;
			}

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D))
			{
				moveDir += player.right;
			}

			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A))
			{
				moveDir -= player.right;
			}

			if (glm::dot(moveDir, moveDir) > 0.f)
			{
				moveDir = glm::normalize(moveDir);
			}

			glm::vec3 deltaMove = moveDir * player.speed * static_cast<float>(dt);

			player.pos += deltaMove;
			deltaMove.y = 0.f;

			player.box = AABB::InitAABB(player.pos, player.halfExtent);

			// Check for wall collisions
			for (auto& wall : stonewalls)
			{
				wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
				if (!player.box.CollideAABB(player.box, wall.box)) continue;
				ResolveCollision(player.pos, player.halfExtent, player.mass, wall.pos, wall.halfExtent, 0.f);
			}

			// Check for table collisions
			for (auto& wall : tables)
			{
				wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
				if (!player.box.CollideAABB(player.box, wall.box)) continue;
				ResolveCollision(player.pos, player.halfExtent, player.mass, wall.pos, wall.halfExtent, 0.f);
			}

			// Check for tree collisions
			for (auto& wall : trees)
			{
				wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
				if (!player.box.CollideAABB(player.box, wall.box)) continue;
				ResolveCollision(player.pos, player.halfExtent, player.mass, wall.pos, wall.halfExtent, 0.f);
			}

			// Check for crate collisions
			for (int i = 0; i < 4; i++)
			{
				crates[i].box = AABB::InitAABB(crates[i].pos, crates[i].halfExtent);
				if (!player.box.CollideAABB(player.box, crates[i].box)) continue;
				ResolveCollision(player.pos, player.halfExtent, player.mass, crates[i].pos, crates[i].halfExtent, crates[i].mass);
			}

			// Check for skeleton collision
			skeleton.box = AABB::InitAABB(skeleton.pos, skeleton.halfExtent);
			if (player.box.CollideAABB(player.box, skeleton.box))
			{
				ResolveCollision(player.pos, player.halfExtent, player.mass, skeleton.pos, skeleton.halfExtent, 0.f);
			}

			player.isMoving = (glm::dot(moveDir, moveDir) > 0.f);
		}
	}
	else
	{
		player.isMoving = false;
	}
	
	player.pos.y = 0.f;
}

void SceneICA2::UpdateMonk(double dt)
{
	if (!inDialogue && !isInteracting && isLanternPuzzleSolved)
	{
		// Finds the direction to the player
		glm::vec3 monkToPlayer = player.pos - monk.pos;
		monkToPlayer.y = 0.f;

		float distSQ = glm::dot(monkToPlayer, monkToPlayer);
		if (distSQ < 0.000001f)
		{
			return;
		}

		float dist = sqrtf(distSQ);
		float rate = 5.f * static_cast<float>(dt);
		if (rate > dist) rate = dist;
		glm::vec3 dir = glm::normalize(monkToPlayer);
		monk.pos += dir * rate;
		
		monk.box = AABB::InitAABB(monk.pos, monk.halfExtent);

		player.box = AABB::InitAABB(player.pos, player.halfExtent);
		if (monk.box.CollideAABB(monk.box, player.box))
		{
			badEndingActive = true;
		}
		
		// Check for wall collisions
		for (auto& wall : stonewalls)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!monk.box.CollideAABB(monk.box, wall.box)) continue;
			ResolveCollision(monk.pos, monk.halfExtent, monk.mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for table collisions
		for (auto& wall : tables)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!monk.box.CollideAABB(monk.box, wall.box)) continue;
			ResolveCollision(monk.pos, monk.halfExtent, monk.mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for tree collisions
		for (auto& wall : trees)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!monk.box.CollideAABB(monk.box, wall.box)) continue;
			ResolveCollision(monk.pos, monk.halfExtent, monk.mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for crate collisions
		for (int i = 0; i < 4; i++)
		{
			crates[i].box = AABB::InitAABB(crates[i].pos, crates[i].halfExtent);
			if (!monk.box.CollideAABB(monk.box, crates[i].box)) continue;
			ResolveCollision(monk.pos, monk.halfExtent, monk.mass, crates[i].pos, crates[i].halfExtent, crates[i].mass);
		}

		// Check for skeleton collision
		skeleton.box = AABB::InitAABB(skeleton.pos, skeleton.halfExtent);
		if (monk.box.CollideAABB(monk.box, skeleton.box))
		{
			ResolveCollision(monk.pos, monk.halfExtent, monk.mass, skeleton.pos, skeleton.halfExtent, 0.f);
		}
	}
}

void SceneICA2::UpdateCrate(double dt)
{
	for (int i = 0; i < 4; i++)
	{
		crates[i].box = AABB::InitAABB(crates[i].pos, crates[i].halfExtent);
		
		// Check for collisions with other crates
		for (int j = i + 1; j < 4; j++)
		{
			crates[j].box = AABB::InitAABB(crates[j].pos, crates[j].halfExtent);
			if (crates[i].box.CollideAABB(crates[i].box, crates[j].box))
			{
				ResolveCollision(crates[i].pos, crates[i].halfExtent, crates[i].mass, crates[j].pos, crates[j].halfExtent, crates[j].mass);
			}
		}

		// Check for collision with target crate location
		for (int j = 0; j < 4; j++)
		{
			AABB targetBox = AABB::InitAABB(crateTargetPos[j], crateTargetHalfExtent);
			if (crates[i].box.CollideAABB(crates[i].box, targetBox))
			{
				if (crates[i].crateNum == j)
				{
					crates[i].pos = crateTargetPos[j];
					crates[i].mass = 0.f;
					isCrateShifted[i] = true;
				}
			}
		}

		// Check for wall collisions
		for (auto& wall : stonewalls)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!crates[i].box.CollideAABB(crates[i].box, wall.box)) continue;
			ResolveCollision(crates[i].pos, crates[i].halfExtent, crates[i].mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for table collisions
		for (auto& wall : tables)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!crates[i].box.CollideAABB(crates[i].box, wall.box)) continue;
			ResolveCollision(crates[i].pos, crates[i].halfExtent, crates[i].mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for tree collisions
		for (auto& wall : trees)
		{
			wall.box = AABB::InitAABB(wall.pos, wall.halfExtent);
			if (!crates[i].box.CollideAABB(crates[i].box, wall.box)) continue;
			ResolveCollision(crates[i].pos, crates[i].halfExtent, crates[i].mass, wall.pos, wall.halfExtent, 0.f);
		}

		// Check for collision with skeleton
		skeleton.box = AABB::InitAABB(skeleton.pos, skeleton.halfExtent);
		if (crates[i].box.CollideAABB(crates[i].box, skeleton.box))
		{
			ResolveCollision(crates[i].pos, crates[i].halfExtent, crates[i].mass, skeleton.pos, skeleton.halfExtent, 0.f);
		}
	}
}

void SceneICA2::UpdateInteractables(double dt)
{
	// Every object that is/has an interactable will appear here
	
	bool inRange = (leftDoor.CheckInteractionDist(player.pos) || rightDoor.CheckInteractionDist(player.pos));
	
	if (inRange && (leftDoor.TriggerInteraction() || rightDoor.TriggerInteraction()))
	{
		if (!isLanternPuzzleSolved)
		{
			// Play locked door dialogue
			speakerDialogue = "You";
			wantToLock = true;
			wantToTarget = glm::vec3((leftDoor.pos.x + rightDoor.pos.x) / 2, -2.f, 50.f);
			StartDialogue(playerDoorLocked);
		}
		else
		{
			// Wait until door opens, and then can get good ending
			if (!hasOpenedDoor)
			{
				hasOpenedDoor = true;
			}
			else if (hasOpenedDoor && leftDoorAngleDeg >= 30.f && rightDoorAngleDeg <= 150.f)
			{
				goodEndingActive = true;
			}
		}
	}

	leftDoor.canInteract = rightDoor.canInteract = inRange;

	// Runs the door opening sequence
	if (hasOpenedDoor)
	{
		
		if (leftDoorAngleDeg >= 60.f)
		{
			leftDoorAngleDeg = 60.f;
		}
		else
		{
			leftDoorAngleDeg += 20.f * static_cast<float>(dt);
		}

		if (rightDoorAngleDeg <= 120.f)
		{
			rightDoorAngleDeg = 120.f;
		}
		else
		{
			rightDoorAngleDeg -= 20.f * static_cast<float>(dt);
		}
	}

	inRange = flashlight.CheckInteractionDist(player.pos);

	if (inRange && flashlight.TriggerInteraction() && !pickedUpFlashlight)
	{
		// Pickup flashlight
		objectiveState++;
		pickedUpFlashlight = true;
		equippedFlashlight = true;
		light[1].power = 2.f;
	}

	flashlight.canInteract = (inRange && !pickedUpFlashlight);

	inRange = chest.CheckInteractionDist(player.pos);

	if (inRange && chest.TriggerInteraction() && isSkeletonSolved && !isChestSolved)
	{
		// Do combination puzzle on chest, will disable interaction once solved
		wantToLock = true;
		wantToTarget = chest.pos;
		isInteracting = true;
		chestMenuActive = true;
		chestNum1 = 0;
		chestNum2 = 0;
		chestNum3 = 0;
	}

	chest.canInteract = (inRange && isSkeletonSolved && !isChestSolved);

	inRange = book.CheckInteractionDist(player.pos);
	if (inRange && book.TriggerInteraction() && isChestSolved)
	{
		// Read a hint on what to do next from the book
		if (!hasReadBookOnce)
		{
			objectiveState++;
			hasReadBookOnce = true;
		}
		
		wantToLock = true;
		wantToTarget = book.pos;
		isInteracting = true;
		bookMenuActive = true;
	}

	book.canInteract = (inRange && isChestSolved);

	for (int i = 0; i < 5; i++)
	{
		inRange = lanterns[i].CheckInteractionDist(player.pos);

		lanterns[i].canInteract = (inRange && !isLanternOn[i] && isChestSolved && !isLanternPuzzleSolved);

		if (!inDialogue && inRange && lanterns[i].TriggerInteraction() && !isLanternOn[i] && isChestSolved && !isLanternPuzzleSolved)
		{
			// Selects lanterns in a specific order
			lanternOrder[lanternOrderIndex] = i;
			lanternOrderIndex++;
			isLanternOn[i] = true;
		}
	}

	if (lanternOrderIndex < 5) return;

	// Checks if order selected by player is correct
	bool isWrongOrder = false;
	for (int i = 0; i < 5; i++)
	{
		if (lanternOrder[i] != correctLanternOrder[i])
		{
			isWrongOrder = true;
			break;
		}
	}

	// Resets entire puzzle when order is wrong after all 5 lanterns lit up
	if (isWrongOrder && !isLanternPuzzleSolved)
	{
		lanternOrderIndex = 0;
		for (int i = 0; i < 5; i++)
		{
			isLanternOn[i] = false;
			lanternOrder[i] = -1;
		}
		return;
	}
	else if (!isWrongOrder && !isLanternPuzzleSolved)
	{
		// Trigger chase sequence by monk
		monk.pos = glm::vec3(0.f, -6.f, 10.f);
		objectiveState++;
		isLanternPuzzleSolved = true;
	}
}

void SceneICA2::UpdateSkeleton(double dt)
{
	bool inRange = skeleton.interaction.CheckInteractionDist(player.pos);

	if (!inDialogue && inRange && skeleton.interaction.TriggerInteraction() && pickedUpFlashlight)
	{
		wantToLock = true;
		wantToTarget = skeleton.pos + glm::vec3(0.f, 5.f, 0.f);

		speakerDialogue = "Skeleton";
		bool allCratesShifted = false;

		switch (skeleton.timesInteracted)
		{
		case 0:
			StartDialogue(skeletonEncounter);
			skeleton.timesInteracted++;
			objectiveState++;

			for (int i = 0; i < 4; i++)
			{
				crates[i].mass = 2.f;
			}
			break;
		case 1:
			if (isCrateShifted[0] && isCrateShifted[1] && isCrateShifted[2] && isCrateShifted[3])
			{
				allCratesShifted = true;
			}

			if (allCratesShifted) 
			{
				isSkeletonSolved = true;
				skeleton.timesInteracted++;
				objectiveState++;
				StartDialogue(skeletonComplete);
			}
			else
			{
				StartDialogue(skeletonEncounterRepeat);
			}

			break;
		case 2:
			StartDialogue(skeletonCompleteRepeat);
			break;
		default:
			break;
		}
	}
	
	skeleton.interaction.canInteract = (inRange && pickedUpFlashlight);
}

void SceneICA2::Render()
{
	// Clear color buffer every frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.position.x + camera.front.x, camera.position.y + camera.front.y, camera.position.z + camera.front.z,
		camera.up.x, camera.up.y, camera.up.z
	);

	// Load identity matrix into the model stack
	modelStack.LoadIdentity();

	RenderLight();

	// Render objects
	//modelStack.PushMatrix();
	//RenderMesh(meshList[GEO_AXES], false);
	//modelStack.PopMatrix();

	RenderSkybox();
	RenderEnvironment();
	RenderPlayer();
	RenderCrate();
	RenderInteractables();
	RenderSkeleton();
	RenderMonk();
	//RenderCollider();
	RenderWorldUI();
	RenderScreenUI();
}

void SceneICA2::RenderMesh(Mesh* mesh, bool enableLight)
{
	//glDisable(GL_CULL_FACE);
	
	glm::mat4 MVP, modelView, modelView_inverse_transpose;

	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, glm::value_ptr(MVP));
	modelView = viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, glm::value_ptr(modelView));
	if (enableLight)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
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
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
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

void SceneICA2::RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey)
{
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_parameters[U_FOG_ENABLED], 0);
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
	glUniform1i(m_parameters[U_FOG_ENABLED], 1);
	glEnable(GL_DEPTH_TEST);
}

void SceneICA2::RenderText(Mesh* mesh, std::string text, glm::vec3 color)
{
	if (!mesh || mesh->textureID <= 0) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_FOG_ENABLED], 0);
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
	glUniform1i(m_parameters[U_FOG_ENABLED], 1);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void SceneICA2::RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y)
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
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_FOG_ENABLED], 0);
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
	glUniform1i(m_parameters[U_FOG_ENABLED], 1);
	projectionStack.PopMatrix();
	viewStack.PopMatrix();
	modelStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void SceneICA2::RenderSkybox()
{
	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, camera.position.z - 100.f);
	RenderMesh(meshList[GEO_FRONT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, camera.position.z + 100.f);
	modelStack.Rotate(180.f, 0, 1, 0);
	RenderMesh(meshList[GEO_BACK], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x + 100.f, camera.position.y, camera.position.z);
	modelStack.Rotate(-90.f, 0, 1, 0);
	RenderMesh(meshList[GEO_RIGHT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x - 100.f, camera.position.y, camera.position.z);
	modelStack.Rotate(90.f, 0, 1, 0);
	RenderMesh(meshList[GEO_LEFT], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y - 100.f, camera.position.z);
	modelStack.Rotate(-90.f, 1, 0, 0);
	modelStack.Rotate(0.f, 0, 0, 1);
	RenderMesh(meshList[GEO_BOTTOM], false);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y + 100.f, camera.position.z);
	modelStack.Rotate(90.f, 1, 0, 0);
	modelStack.Rotate(0.f, 0, 0, 1);
	RenderMesh(meshList[GEO_TOP], false);
	modelStack.PopMatrix();
}

void SceneICA2::RenderLight()
{
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		if (i == 1 && !equippedFlashlight)
		{
			// Don't render light if player hasn't equipped flashlight
		}
		else
		{
			modelStack.PushMatrix();
			modelStack.Translate(light[i].position.x, light[i].position.y, light[i].position.z);
			modelStack.Scale(0.1f, 0.1f, 0.1f);

			RenderMesh(meshList[GEO_SPHERE], false);
			modelStack.PopMatrix();
		}
	}
}

void SceneICA2::RenderPlayer()
{
	if (equippedFlashlight)
	{
		modelStack.PushMatrix();
		modelStack.Translate(flashlightPos.x, flashlightPos.y, flashlightPos.z);
		modelStack.Rotate(-camera.yaw + 180, 0, 1, 0);
		modelStack.Rotate(-camera.pitch, 0, 0, 1);
		modelStack.Scale(0.05f, 0.05f, 0.05f);
		RenderMesh(meshList[GEO_FLASHLIGHT], true);
		modelStack.PopMatrix();
	}
}

void SceneICA2::RenderSkeleton()
{
	modelStack.PushMatrix();
	modelStack.Translate(skeleton.pos.x, skeleton.pos.y, skeleton.pos.z);
	modelStack.Scale(3.f, 3.f, 3.f);
	RenderMesh(meshList[GEO_SKELETON], true);
	modelStack.PopMatrix();
}

void SceneICA2::RenderMonk()
{
	modelStack.PushMatrix();
	ApplyObjFacePlayer(monk.pos, player.pos); // Has translate and rotate
	modelStack.Scale(6.f, 6.f, 6.f);
	RenderMesh(meshList[GEO_MONK], true);
	modelStack.PopMatrix();
}

void SceneICA2::RenderCrate()
{
	for (int i = 0; i < 4; i++)
	{
		modelStack.PushMatrix();
		modelStack.Translate(crates[i].pos.x, crates[i].pos.y, crates[i].pos.z);
		modelStack.Scale(1.5f, 1.5f, 1.5f);
		RenderMesh(meshList[GEO_CRATE], true);
		modelStack.PopMatrix();
	}
}

void SceneICA2::RenderInteractables()
{
	modelStack.PushMatrix();
	modelStack.Translate(leftDoor.pos.x, leftDoor.pos.y, leftDoor.pos.z);
	modelStack.Rotate(leftDoorAngleDeg, 0, 1, 0);
	modelStack.Scale(0.03f, 0.03f, 0.03f);
	RenderMesh(meshList[GEO_DOOR], true);
	modelStack.PopMatrix();

	modelStack.PushMatrix();
	modelStack.Translate(rightDoor.pos.x, rightDoor.pos.y, rightDoor.pos.z);
	modelStack.Rotate(rightDoorAngleDeg, 0, 1, 0);
	modelStack.Scale(0.03f, 0.03f, 0.03f);
	RenderMesh(meshList[GEO_DOOR], true);
	modelStack.PopMatrix();
	
	modelStack.PushMatrix();
	modelStack.Translate(chest.pos.x, chest.pos.y, chest.pos.z);
	modelStack.Rotate(0.f, 0, 1, 0);
	modelStack.Scale(2.5f, 2.5f, 2.5f);
	RenderMesh(meshList[GEO_CHEST], true);
	modelStack.PopMatrix();

	if (!pickedUpFlashlight)
	{
		modelStack.PushMatrix();
		modelStack.Translate(flashlight.pos.x, flashlight.pos.y, flashlight.pos.z);
		modelStack.Rotate(45.f, 0, 1, 0);
		modelStack.Scale(0.05f, 0.05f, 0.05f);
		RenderMesh(meshList[GEO_FLASHLIGHT], true);
		modelStack.PopMatrix();
	}

	if (isChestSolved)
	{
		modelStack.PushMatrix();
		modelStack.Translate(book.pos.x, book.pos.y, book.pos.z);
		modelStack.Rotate(135.f, 0, 1, 0);
		modelStack.Scale(0.3f, 0.3f, 0.3f);
		RenderMesh(meshList[GEO_BOOK], true);
		modelStack.PopMatrix();
	}

	for (int i = 0; i < 5; i++)
	{
		modelStack.PushMatrix();
		modelStack.Translate(lanterns[i].pos.x, lanterns[i].pos.y, lanterns[i].pos.z);
		modelStack.Rotate(lanternAngleDeg[i], 0, 1, 0);
		modelStack.Scale(2.f, 2.f, 2.f);
		if (isLanternOn[i])
		{
			RenderMesh(meshList[GEO_LANTERN_LIT], true);
		}
		else
		{
			RenderMesh(meshList[GEO_LANTERN_UNLIT], true);
		}


		modelStack.PopMatrix();
	}
}

void SceneICA2::RenderEnvironment()
{
	for (auto& wall : stonewalls)
	{
		modelStack.PushMatrix();
		modelStack.Translate(wall.pos.x, wall.pos.y, wall.pos.z);
		modelStack.PushMatrix();
		modelStack.Scale(wall.halfExtent.x * 2.f, 4.f, wall.halfExtent.y * 2.f);
		modelStack.PopMatrix();
		modelStack.Scale(6.f, 6.f, 6.f);
		for (int i = 0; i < 9; i++)
		{
			modelStack.PushMatrix();
			if (wall.pos.x > 0.f)
			{
				modelStack.Translate(0.f, 0.f, 8.5f - (i * 2.2f));
				modelStack.Rotate(90.f, 0, 1, 0);
			}
			else if (wall.pos.x < 0.f)
			{
				modelStack.Translate(0.f, 0.f, -9.f + (i * 2.2f));
				modelStack.Rotate(90.f, 0, 1, 0);
			}
			
			if (wall.pos.z > 0.f)
			{
				if (i > 3)
				{
					modelStack.Translate(8.5f - (i * 2.2f) - 1.25f, 0.f, 0.f);
				}
				else
				{
					modelStack.Translate(8.5f - (i * 2.2f), 0.f, 0.f);
				}
			}
			else if (wall.pos.z < 0.f)
			{
				modelStack.Translate(-9.f + (i * 2.2f), 0.f, 0.f);
			}
			RenderMesh(meshList[GEO_STONEWALL], true);
			modelStack.PopMatrix();
		}
		
		modelStack.PopMatrix();
	}

	for (auto& table : tables)
	{
		modelStack.PushMatrix();
		modelStack.Translate(table.pos.x, table.pos.y, table.pos.z);
		modelStack.Rotate(0.f, 1, 0, 0);
		modelStack.Scale(5.f, 5.f, 5.f);
		RenderMesh(meshList[GEO_TABLE], true);
		modelStack.PopMatrix();
	}

	for (auto& tree : trees)
	{
		modelStack.PushMatrix();
		modelStack.Translate(tree.pos.x, tree.pos.y, tree.pos.z);
		modelStack.Rotate(0.f, 1, 0, 0);
		modelStack.Scale(0.1f, 0.1f, 0.1f);
		RenderMesh(meshList[GEO_TREE], true);
		modelStack.PopMatrix();
	}
}

void SceneICA2::RenderCollider()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Stone walls
	for (auto& wall : stonewalls)
	{
		modelStack.PushMatrix();
		modelStack.Translate(wall.pos.x, wall.pos.y, wall.pos.z);
		modelStack.Scale(wall.halfExtent.x * 2.f, 4.f, wall.halfExtent.y * 2.f);
		RenderMesh(meshList[GEO_CUBE], true);
		modelStack.PopMatrix();
	}

	// Crates
	for (int i = 0; i < 4; i++)
	{
		modelStack.PushMatrix();
		modelStack.Translate(crates[i].pos.x, crates[i].pos.y, crates[i].pos.z);
		modelStack.Scale(crates[i].halfExtent.x * 2.f, 1.f, crates[i].halfExtent.y * 2.f);
		RenderMesh(meshList[GEO_CUBE], true);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(crateTargetPos[i].x, crateTargetPos[i].y, crateTargetPos[i].z);
		modelStack.Scale(crateTargetHalfExtent.x, 1.f, crateTargetHalfExtent.y);
		RenderMesh(meshList[GEO_CUBE], true);
		modelStack.PopMatrix();
	}

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //default fill mode
}

void SceneICA2::RenderWorldUI()
{
	// Order of render:
	// First: Always appear in front of others
	// Last: Always appear behind others
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glUniform1i(m_parameters[U_FOG_ENABLED], 0);

	modelStack.PushMatrix();
	modelStack.Translate(0.f, 50.f, 0.f);
	modelStack.Rotate(90.f, 1, 0, 0);
	modelStack.Scale(3, 3, 3);
	RenderMesh(meshList[GEO_2D_MOON], false);
	modelStack.PopMatrix();
	
	if (leftDoor.canInteract && rightDoor.canInteract)
	{
		modelStack.PushMatrix();
		ApplyObjFacePlayer(glm::vec3((leftDoor.pos.x + rightDoor.pos.x) / 2, worldUIBob + 2.f, 50.f), player.pos); // Has translate and rotate
		modelStack.Scale(3, 3, 3);
		RenderMesh(meshList[GEO_2D_INTERACT], false);
		modelStack.PopMatrix();
	}

	if (flashlight.canInteract)
	{
		modelStack.PushMatrix();
		ApplyObjFacePlayer(flashlight.pos + glm::vec3(0.f, worldUIBob + 2.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(3, 3, 3);
		RenderMesh(meshList[GEO_2D_INTERACT], false);
		modelStack.PopMatrix();
	}

	if (chest.canInteract)
	{
		modelStack.PushMatrix();
		ApplyObjFacePlayer(chest.pos + glm::vec3(0.f, worldUIBob + 3.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(3, 3, 3);
		RenderMesh(meshList[GEO_2D_INTERACT], false);
		modelStack.PopMatrix();
	}

	if (book.canInteract)
	{
		modelStack.PushMatrix();
		ApplyObjFacePlayer(book.pos + glm::vec3(0.f, worldUIBob + 2.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(3, 3, 3);
		RenderMesh(meshList[GEO_2D_INTERACT], false);
		modelStack.PopMatrix();
	}

	if (skeleton.interaction.canInteract)
	{
		modelStack.PushMatrix();
		ApplyObjFacePlayer(skeleton.pos + glm::vec3(0.f, worldUIBob + 6.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(1, 1, 1);
		RenderMesh(meshList[GEO_2D_CHAT], false);
		modelStack.PopMatrix();
	}

	for (int i = 0; i < 5; i++)
	{
		if (lanterns[i].canInteract)
		{
			modelStack.PushMatrix();
			ApplyObjFacePlayer(light[i + 2].position + glm::vec3(0.f, worldUIBob + 2.5f, 0.f), player.pos); // Has translate and rotate
			modelStack.Scale(2, 2, 2);
			RenderMesh(meshList[GEO_2D_INTERACT], false);
			modelStack.PopMatrix();
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (!(!isCrateShifted[i] && objectiveState == 2)) continue;
		
		modelStack.PushMatrix();
		ApplyObjFacePlayer(crates[i].pos + glm::vec3(0.f, worldUIBob + 4.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(1, 1, 1);
		RenderText(meshList[GEO_TEXT], std::to_string(i), glm::vec3(1, 1, 1));
		modelStack.PopMatrix();
	}

	for (int i = 0; i < 4; i++)
	{
		if (!(!isCrateShifted[i] && objectiveState == 2)) continue;
		
		modelStack.PushMatrix();
		ApplyObjFacePlayer(crateTargetPos[i] + glm::vec3(0.f, worldUIBob + 6.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(1, 1, 1);
		RenderText(meshList[GEO_TEXT], std::to_string(i), glm::vec3(1, 1, 1));
		modelStack.PopMatrix();
	}

	glDisable(GL_BLEND);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < 4; i++)
	{
		if (!(!isCrateShifted[i] && objectiveState == 2)) continue;
		
		modelStack.PushMatrix();
		ApplyObjFacePlayer(crateTargetPos[i] + glm::vec3(0.f, worldUIBob + 5.f, 0.f), player.pos); // Has translate and rotate
		modelStack.Scale(1, 1, 1);
		RenderMesh(meshList[GEO_2D_MAP], false);
		modelStack.PopMatrix();
	}

	glUniform1i(m_parameters[U_FOG_ENABLED], 1);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void SceneICA2::RenderScreenUI()
{
	if (inCutscene) RenderMeshOnScreen(meshList[GEO_PLANE], 800, 900, 1600, 300);
	glm::vec3 color = glm::vec3(1.f, 1.f, 1.f);
	if (objectiveState == 5) color = glm::vec3(1.f, 0.f, 0.f);

	if (!inCutscene)
	{
		if (!inDialogue && !isInteracting)
		{
			RenderTextOnScreen(meshList[GEO_TEXT], "Objective: " + objectiveText, color, 40, 20, 850);
			std::string temp("FPS:" + std::to_string(fps));
			RenderTextOnScreen(meshList[GEO_TEXT], temp.substr(0, 9), glm::vec3(0, 1, 0), 40, 20, 800);
		}
		
		if ((leftDoor.canInteract || rightDoor.canInteract || flashlight.canInteract || chest.canInteract || book.canInteract || skeleton.interaction.canInteract || lanterns[0].canInteract || lanterns[1].canInteract || lanterns[2].canInteract || lanterns[3].canInteract || lanterns[4].canInteract) && !inDialogue && !isInteracting && !inCutscene)
		{
			RenderTextOnScreen(meshList[GEO_TEXT], "Press [E]", glm::vec3(1, 1, 1), 30, 1200, 200);
			RenderTextOnScreen(meshList[GEO_TEXT], "to interact", glm::vec3(1, 1, 1), 30, 1200, 150);
		}

		if (pickedUpFlashlight && equippedFlashlight && !inDialogue && !isInteracting && !inCutscene)
		{
			RenderTextOnScreen(meshList[GEO_TEXT], "Press [1] to", glm::vec3(1, 1, 1), 30, 1200, 800);
			RenderTextOnScreen(meshList[GEO_TEXT], "swap in/out", glm::vec3(1, 1, 1), 30, 1200, 750);
			RenderTextOnScreen(meshList[GEO_TEXT], "flashlight", glm::vec3(1, 1, 1), 30, 1200, 700);

			RenderTextOnScreen(meshList[GEO_TEXT], "Left click to", glm::vec3(1, 1, 1), 30, 1200, 600);
			RenderTextOnScreen(meshList[GEO_TEXT], "turn on/off", glm::vec3(1, 1, 1), 30, 1200, 550);
			RenderTextOnScreen(meshList[GEO_TEXT], "flashlight", glm::vec3(1, 1, 1), 30, 1200, 500);
		}
	}

	RenderMenu();
	RenderDialogue();
}

void SceneICA2::RenderMenu()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if (chestMenuActive)
	{
		RenderMeshOnScreen(meshList[GEO_PLANE], 800, 200, 500, 200);
		RenderMeshOnScreen(meshList[GEO_2D_KEYLOCK], 800, 600, 600, 600);
		RenderMeshOnScreen(meshList[GEO_2D_RIGHTARROW], 1090, 200, 150, 150);
		RenderMeshOnScreen(meshList[GEO_2D_UPARROW], 650, 250, 100, 100);
		RenderMeshOnScreen(meshList[GEO_2D_DOWNARROW], 650, 150, 100, 100);
		RenderMeshOnScreen(meshList[GEO_2D_UPARROW], 800, 250, 100, 100);
		RenderMeshOnScreen(meshList[GEO_2D_DOWNARROW], 800, 150, 100, 100);
		RenderMeshOnScreen(meshList[GEO_2D_UPARROW], 950, 250, 100, 100);
		RenderMeshOnScreen(meshList[GEO_2D_DOWNARROW], 950, 150, 100, 100);

		RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(chestNum1), glm::vec3(1, 1, 1), 40, 635, 180);
		RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(chestNum2), glm::vec3(1, 1, 1), 40, 790, 180);
		RenderTextOnScreen(meshList[GEO_TEXT], std::to_string(chestNum3), glm::vec3(1, 1, 1), 40, 945, 180);
		RenderTextOnScreen(meshList[GEO_TEXT], "Press [E] to Exit", glm::vec3(1.f, 1.f, 1.f), 40.f, 480.f, 10.f);
	}

	if (bookMenuActive)
	{
		RenderMeshOnScreen(meshList[GEO_2D_PAGE], 800, 500, 600, 600);
		RenderTextOnScreen(meshList[GEO_TEXT], "Press [E] to Exit", glm::vec3(1.f, 1.f, 1.f), 40.f, 480.f, 10.f);
	}

	if (goodEndingActive)
	{
		RenderMeshOnScreen(meshList[GEO_PLANE], 800, 450, 1600, 900);
		RenderMeshOnScreen(meshList[GEO_2D_GOOD], 800, 500, 600, 600);
	}

	if (badEndingActive)
	{
		RenderMeshOnScreen(meshList[GEO_PLANE], 800, 450, 1600, 900);
		RenderMeshOnScreen(meshList[GEO_2D_BAD], 800, 500, 600, 600);
	}

	glDisable(GL_BLEND);
}

void SceneICA2::RenderDialogue()
{
	if (!inDialogue || currentDialogue == nullptr) return;
	if (dialogueIndex < 0 || dialogueIndex >= static_cast<int>(currentDialogue->size())) return;

	const std::string& line = (*currentDialogue)[dialogueIndex];

	// Dialogue background
	RenderMeshOnScreen(meshList[GEO_PLANE], 800, 0, 1600, 300);
	

	// Speaker
	RenderTextOnScreen(meshList[GEO_TEXT], speakerDialogue, glm::vec3(1.f, 1.f, 1.f), 30.f, 700.f, 110.f);

	// Main dialogue
	RenderTextOnScreen(meshList[GEO_TEXT], line, glm::vec3(1.f, 1.f, 1.f), 40.f, 200.f, 50.f);

	// Continue hint
	RenderTextOnScreen(meshList[GEO_TEXT], "Press [SPACE] to Continue", glm::vec3(1.f, 1.f, 1.f), 25.f, 500.f, 10.f);
}

void SceneICA2::HandleKeyPress()
{
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

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_Z))
	{
		for (int i = 0; i < 4; i++)
		{
			crates[i].pos = crateTargetPos[i];
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_X))
	{
		if (!isLanternPuzzleSolved)
		{
			for (int i = 0; i < 5; i++)
			{
				lanternOrder[i] = correctLanternOrder[i];
				isLanternOn[i] = true;
			}

			isLanternPuzzleSolved = true;
			objectiveState++;
			monk.pos = glm::vec3(0.f, -6.f, 10.f);
		}
	}

	if (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_1) && pickedUpFlashlight)
	{
		if (equippedFlashlight)
		{
			equippedFlashlight = false;
			light[1].power = 0.01f;
		}
		else
		{
			equippedFlashlight = true;
			light[1].power = 2.f;
		}
	}

}

void SceneICA2::HandleMouseInput()
{
	static bool isLeftUp = false;
	static bool isRightUp = false;

	// Process Left Button
	if (!isLeftUp && MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !inDialogue && !isInteracting)
	{
		if (!inDialogue && !isInteracting && equippedFlashlight)
		{
			if (light[1].power == 0.01f)
			{
				light[1].power = 2.f;
			}
			else
			{
				light[1].power = 0.01f;
			}
		}
		
		isLeftUp = true;
	}
	else if (isLeftUp && MouseController::GetInstance()->IsButtonUp(GLFW_MOUSE_BUTTON_LEFT) && !inDialogue && !isInteracting)
	{
		isLeftUp = false;
	}
}

void SceneICA2::Exit()
{
	// Cleanup VBO here
	for (int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if (meshList[i])
		{
			delete meshList[i];
		}
	}
	glDeleteVertexArrays(1, &m_vertexArrayID);
	glDeleteProgram(m_programID);
}