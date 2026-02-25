#ifndef SCENE_MEDICAL_H
#define SCENE_MEDICAL_H

#include "BaseScene.h"


class SceneMedical : public BaseScene
{
public:

	enum GEOMETRY_TYPE : int
	{
		AXES = 0,
		GROUND,
		SKYBOX,
		LIGHT,
		GROUP,

		FONT_CASCADIA_MONO,

		// SceneMedical Variables
		NANOBOT_MODEL,
		BACTERIA_MODEL,
		VIRUS_MODEL,

		ENV_SKYBOX,
		ENV_WALL,
		ENV_SPHERE_MODEL,
		ENV_BLOCK_MODEL,
		ENV_STRING_MODEL,
		ENV_LIQUID_MODEL,
		ENV_LIQUID_FLAT_MODEL,

		GAME_CROSSHAIR,
		GAME_UI_BASE,
		GAME_UI_PLATE,
		GAME_OVERLOADSTACK_G,
		GAME_OVERLOADSTACK_Y,
		GAME_OVERLOADSTACK_R,

		TOTAL
	};

	SceneMedical();
	~SceneMedical();

	void Init() override;
	void Update(double dt) override;
	void Render() override;
	void Exit() override;

private:

	enum SFX_TYPE {
		GOOFY_AHH_ASRIEL_STAR_SOUND,

		TOTAL_SFX
	};

	float FontSpacing(GEOMETRY_TYPE font) {
		switch (font) {
		case GEOMETRY_TYPE::FONT_CASCADIA_MONO: return 0.375f;
		default: return 1;
		}
	}

	void HandleKeyPress();

	void RenderMesh(GEOMETRY_TYPE type, bool enableLight);
	void RenderObj(const std::shared_ptr<RenderObject> obj);

	// debug
	bool debug = false;

	std::vector<std::weak_ptr<RenderObject>> debugTextList;
	void InitDebugText(GEOMETRY_TYPE font);
	// if passed in index, overwrite data in that specific debug text
	// returns success
	// does not work in Init()
	bool AddDebugText(const std::string& text, int index = -1);
	void ClearDebugText();

	bool cullFaceActive = true;
	bool wireFrameActive = false;

	bool renderDebugPhysics = false;
	Mesh* debugPhysicsWorld;
	double debugPhysicsTimer = 0;
	

	// ***************************************************************
	// Medical Gun Game Scene Specifics
	// ***************************************************************
	// Game State Related
	// ***************************************************************
	enum class MedicalGameState
	{
		PLAYING,
		WON,
		RESULTS
	};
	MedicalGameState currentState = MedicalGameState::PLAYING;
	bool isInResults = false;



	// ***************************************************************
	// Overload Mechanic Related
	// ***************************************************************
	struct Overload
	{
		std::shared_ptr<RenderObject> object;
	};
	std::vector<Overload> overloadStackUI;
	int overloadingStack = 0;
	int maxOverload = 5;
	float overloadCoolTimer = 0.0f; // Use only if overloading stack is above 3
	bool changeInOverloadStack = false;



	// ***************************************************************
	// Menus Related
	// ***************************************************************
	struct GameMenu
	{
		std::shared_ptr<RenderObject> object;
	};
	std::vector<GameMenu> menus;
	std::vector<std::shared_ptr<TextObject>> textObjects;
	bool isHelpOpen = true;
	bool menuChange = true;



	// ***************************************************************
	// Enemy Entities Related
	// ***************************************************************
	int maxEntitiesP = 10;
	int maxEntitiesAI = 5;
	int currentSpawningP = 0;
	int currentSpawningAI = 0;
	int remainingEntitiesP = 0;
	int remainingEntitiesAI = 0;

	float bacteriaSpawnTimer = 0.0f;
	float bacteriaSpawnInterval = 3.0f;
	float virusSpawnTimer = 0.0f;
	float virusSpawnInterval = 6.0f;



	// ***************************************************************
	// Enemy: Bacteria Related
	// ***************************************************************
	struct Bacteria
	{
		std::shared_ptr<RenderObject> object;

		int bacteriaHP = 2;
		float bacteriaInvulnerabilityTimer = 0.0f;

		float bacteriaDivertTimer = 0.0f;
		glm::vec3 patPoint = glm::vec3(0);
	};

	std::vector<Bacteria> bacterias;



	// ***************************************************************
	// Enemy: Virus Related
	// ***************************************************************
	struct Virus
	{
		std::shared_ptr<RenderObject> object;
		
		int virusHP = 3;
		float virusInvulnerabilityTimer = 0.0f;
	};

	std::vector<Virus> viruses;



	// ***************************************************************
	// Nanobot Related
	// ***************************************************************
	struct Nanobot
	{
		std::shared_ptr<RenderObject> object;
		float lifetime = 5.0f;
	};

	bool isNanobotFired = false;
	int maxNanobotAmmo = 15;
	int currentActiveNanobotAmmo = 0;
	std::vector<Nanobot> nanobots;



	// ***************************************************************
	// Wave and Win Condition Related
	// ***************************************************************
	int waveNumber = 1; // default wave at 1
	int waveTimeLeft = 180; // 3 minutes per wave
	float waveTimeAccumulator = 0.0f;
	int totalTimeTaken = 0;
	int bestTimeTaken = 0;

	std::string gameGrade = "D"; // Use if necessary for integration based on Aizzul's suggestion of average grade

	bool isGameReset = false;
	void HandleWinCondition();
	void ChangeWave(int waveNumber);



	// ***************************************************************
	// Render Helpers for On-Screen UI
	// ***************************************************************
	void ShowOverloadStack();
	void ShowHelpMenu();
	void ClearHelpMenu();
	void ShowResultsMenu();
	void ClearResultsMenu();



	// ***************************************************************
	// Text Related
	// ***************************************************************
	std::shared_ptr<TextObject> AddFlexText(const std::string& name, const std::string& text, glm::vec3 pos, glm::vec3 scl, GEOMETRY_TYPE font);
	std::vector<std::weak_ptr<RenderObject>> sceneMedicalTextList;
	void InitSceneMedicalText(GEOMETRY_TYPE font);
	bool AddSceneMedicalText(const std::string& text, int index = -1);
	void ClearSceneMedicalText();
};

#endif