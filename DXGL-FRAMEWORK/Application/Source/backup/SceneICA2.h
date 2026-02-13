#ifndef SCENE_ICA2_H
#define SCENE_ICA2_H

#include <iostream>
#include <vector>
#include <string>
#include "Scene.h"
#include "Mesh.h"
#include "FPCamera.h"
#include "MatrixStack.h"
#include "Light.h"
#include "MouseController.h"
#include "ICA2Struct.h"

class SceneICA2 : public Scene
{
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_SPHERE,
		GEO_CUBE,
		GEO_LEFT,
		GEO_RIGHT,
		GEO_TOP,
		GEO_BOTTOM,
		GEO_FRONT,
		GEO_BACK,
		GEO_PLANE,
		GEO_TEXT,

		GEO_STONEWALL,
		GEO_TREE,
		GEO_TABLE,
		GEO_LANTERN_UNLIT,
		GEO_LANTERN_LIT,
		GEO_DOOR,
		
		GEO_SKELETON,
		GEO_MONK,
		GEO_FLASHLIGHT,
		GEO_BOOK,
		GEO_CRATE,
		GEO_CHEST,

		GEO_2D_INTERACT,
		GEO_2D_CHAT,
		GEO_2D_MAP,
		GEO_2D_PAGE,
		GEO_2D_GOOD,
		GEO_2D_BAD,
		GEO_2D_KEYLOCK,
		GEO_2D_RIGHTARROW,
		GEO_2D_UPARROW,
		GEO_2D_DOWNARROW,
		GEO_2D_MOON,
		
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

		U_LIGHT1_TYPE,
		U_LIGHT1_POSITION,
		U_LIGHT1_COLOR,
		U_LIGHT1_POWER,
		U_LIGHT1_KC,
		U_LIGHT1_KL,
		U_LIGHT1_KQ,
		U_LIGHT1_SPOTDIRECTION,
		U_LIGHT1_COSCUTOFF,
		U_LIGHT1_COSINNER,
		U_LIGHT1_EXPONENT,

		U_LIGHT2_TYPE,
		U_LIGHT2_POSITION,
		U_LIGHT2_COLOR,
		U_LIGHT2_POWER,
		U_LIGHT2_KC,
		U_LIGHT2_KL,
		U_LIGHT2_KQ,
		U_LIGHT2_SPOTDIRECTION,
		U_LIGHT2_COSCUTOFF,
		U_LIGHT2_COSINNER,
		U_LIGHT2_EXPONENT,

		U_LIGHT3_TYPE,
		U_LIGHT3_POSITION,
		U_LIGHT3_COLOR,
		U_LIGHT3_POWER,
		U_LIGHT3_KC,
		U_LIGHT3_KL,
		U_LIGHT3_KQ,
		U_LIGHT3_SPOTDIRECTION,
		U_LIGHT3_COSCUTOFF,
		U_LIGHT3_COSINNER,
		U_LIGHT3_EXPONENT,

		U_LIGHT4_TYPE,
		U_LIGHT4_POSITION,
		U_LIGHT4_COLOR,
		U_LIGHT4_POWER,
		U_LIGHT4_KC,
		U_LIGHT4_KL,
		U_LIGHT4_KQ,
		U_LIGHT4_SPOTDIRECTION,
		U_LIGHT4_COSCUTOFF,
		U_LIGHT4_COSINNER,
		U_LIGHT4_EXPONENT,

		U_LIGHT5_TYPE,
		U_LIGHT5_POSITION,
		U_LIGHT5_COLOR,
		U_LIGHT5_POWER,
		U_LIGHT5_KC,
		U_LIGHT5_KL,
		U_LIGHT5_KQ,
		U_LIGHT5_SPOTDIRECTION,
		U_LIGHT5_COSCUTOFF,
		U_LIGHT5_COSINNER,
		U_LIGHT5_EXPONENT,

		U_LIGHT6_TYPE,
		U_LIGHT6_POSITION,
		U_LIGHT6_COLOR,
		U_LIGHT6_POWER,
		U_LIGHT6_KC,
		U_LIGHT6_KL,
		U_LIGHT6_KQ,
		U_LIGHT6_SPOTDIRECTION,
		U_LIGHT6_COSCUTOFF,
		U_LIGHT6_COSINNER,
		U_LIGHT6_EXPONENT,

		U_NUMLIGHTS,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_LIGHTENABLED,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,
		U_FOG_ENABLED,
		U_FOG_COLOR,
		U_FOG_DENSITY,
		U_FOG_MINDIST,
		U_FOG_MAXDIST,
		U_TOTAL,
	};

	SceneICA2();
	~SceneICA2();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:
	// Helper functions
	void SetMouseCapture(bool enable);
	void ResolveCollision(glm::vec3& box1Pos, const glm::vec2& box1HalfExtent, float box1Mass, glm::vec3& box2Pos, const glm::vec2& box2HalfExtent, float box2Mass);
	void ApplyObjFacePlayer(const glm::vec3& objPos, const glm::vec3& playerPos, float offsetDeg = 90.f);
	void StartDialogue(std::vector<std::string>& dialogue);
	void AdvanceDialogue();
	void EndDialogue();
	void IntroCutscene();
	void MonkCutscene();
	void RunChestMenu();
	
	// Init functions
	void InitUniforms();
	void InitModels();
	void InitGame();
	void InitLight();
	void InitPlayer();
	void InitSkeleton();
	void InitMonk();
	void InitCrate();
	void InitInteractables();
	void InitEnvironment();
	
	// Update functions
	void UpdateGame(double dt);
	void UpdateLight(double dt);
	void UpdatePOV(double dt);
	void UpdatePlayer(double dt);
	void UpdateMonk(double dt);
	void UpdateCrate(double dt);
	void UpdateInteractables(double dt);
	void UpdateSkeleton(double dt);
	
	// Render functions
	void RenderSkybox();
	void RenderLight();
	void RenderPlayer();
	void RenderSkeleton();
	void RenderMonk();
	void RenderCrate();
	void RenderInteractables();
	void RenderEnvironment();
	void RenderCollider();
	void RenderWorldUI();
	void RenderScreenUI();
	void RenderMenu();
	void RenderDialogue();

	// Render helper functions
	void RenderMesh(Mesh* mesh, bool enableLight);
	void RenderMeshOnScreen(Mesh* mesh, float x, float y, float sizex, float sizey);
	void RenderText(Mesh* mesh, std::string text, glm::vec3 color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, glm::vec3 color, float size, float x, float y);

	// HandleInput functions
	void HandleKeyPress();
	void HandleMouseInput();
	
	// Geometry/Shader members
	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];

	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	// Matrix Stack & projection members
	MatrixStack modelStack, viewStack, projectionStack;
	int projType = 1; // fix to 0 for orthographic, 1 for projection

	// Game members
	// -> Game manager
	float fps;
	bool mouseCaptured = false;
	double elapsedTime;
	float worldUIBob;
	int objectiveState;
	std::string objectiveText;
	bool goodEndingActive;
	bool badEndingActive;

	// Cutscene members
	bool inCutscene;

	// -> Intro
	int introIndex;

	// -> Monk appearance
	int monkIndex;

	// Dialogue members
	std::string speakerDialogue;
	bool inDialogue;
	int dialogueIndex;
	std::vector<std::string>* currentDialogue;

	// Player members
	Player player;
	FPCamera camera;
	bool wantToLock;
	bool equippedFlashlight;
	glm::vec3 wantToTarget{};
	glm::vec3 flashlightPos{};
	std::vector<std::string> playerWokeUp;
	std::vector<std::string> playerLooking;
	std::vector<std::string> playerReady;
	std::vector<std::string> playerDoorLocked;
	std::vector<std::string> playerMonkFirst;
	std::vector<std::string> playerMonkSecond;
	
	// Environment members
	// -> Light
	static const int NUM_LIGHTS = 7;
	Light light[NUM_LIGHTS];
	bool enableLight;

	// -> Fog
	Fog fog;
	bool enableFog = true;
	
	// -> Walls
	std::vector<Wall> stonewalls;

	// -> Tables
	std::vector<Wall> tables;

	// -> Trees
	std::vector<Wall> trees;

	// Skeleton
	Skeleton skeleton;
	bool isSkeletonSolved;
	std::vector<std::string> skeletonEncounter;
	std::vector<std::string> skeletonEncounterRepeat;
	std::vector<std::string> skeletonComplete;
	std::vector<std::string> skeletonCompleteRepeat;

	// Monk
	Monk monk;
	std::vector<std::string> monkFirst;
	std::vector<std::string> monkSecond;
	std::vector<std::string> monkThird;

	// Crates
	Crate crates[4];
	glm::vec3 crateTargetPos[4];
	glm::vec2 crateTargetHalfExtent;
	bool isCrateShifted[4];

	// Interactive members
	// -> Doors
	Interactable leftDoor;
	Interactable rightDoor;
	float leftDoorAngleDeg;
	float rightDoorAngleDeg;
	bool hasOpenedDoor;

	// -> Flashlight
	bool isInteracting;
	Interactable flashlight;
	bool pickedUpFlashlight;

	// -> Chest
	Interactable chest;
	bool chestMenuActive;
	bool isChestSolved;
	int chestNum1;
	int chestNum2;
	int chestNum3;

	// -> Book
	Interactable book;
	bool bookMenuActive;
	bool hasReadBookOnce;

	// -> Lanterns
	Interactable lanterns[5];
	float lanternAngleDeg[5];
	bool isLanternOn[5];
	int lanternOrder[5];
	const int correctLanternOrder[5] = { 1, 3, 0, 2, 4 };
	int lanternOrderIndex;
	bool isLanternPuzzleSolved;
};

#endif