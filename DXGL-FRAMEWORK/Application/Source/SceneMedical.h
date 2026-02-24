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

		// add more variables here
		FLASHLIGHT,

		UI_TEST,
		UI_TEST_2,

		PNG_TEST,

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
		GAME_OVERLOADSTACK_BASE,
		GAME_OVERLOADSTACK_PLATE,
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
	
	// Medical Gun Game Scene Specifics
	int waveNumber = 1; // default wave at 1
	int waveTimeLeft = 180; // 3 minutes per wave
	float waveTimeAccumulator = 0.0f;

	int maxEntitiesP = 10;
	int maxEntitiesAI = 5;
	int currentSpawningP = 0;
	int currentSpawningAI = 0;
	int remainingEntitiesP = 10;
	int remainingEntitiesAI = 5;

	int overloadingStack = 0;
	int maxOverload = 5;
	float overloadCoolTimer = 0.0f; // Use only if overloading stack is above 3
	bool changeInOverloadStack = false;
	int timesOverloaded = 0; // only use if not enough time to implement losing and resetting to at least base wave

	float bacteriaSpawnTimer = 0.0f;
	float bacteriaSpawnInterval = 3.0f;
	float virusSpawnTimer = 0.0f;
	float virusSpawnInterval = 6.0f;

	struct Bacteria
	{
		std::shared_ptr<RenderObject> object;

		int bacteriaHP = 2;
		float bacteriaInvulnerabilityTimer = 0.0f;

		float bacteriaDivertTimer = 0.0f;
		glm::vec3 patPoint;
	};

	std::vector<Bacteria> bacterias;

	struct Virus
	{
		std::shared_ptr<RenderObject> object;
		
		int virusHP = 3;
		float virusInvulnerabilityTimer = 0.0f;
	};

	std::vector<Virus> viruses;

	struct Nanobot
	{
		std::shared_ptr<RenderObject> object;
		float lifetime;
	};

	bool isNanobotFired = false;
	int maxNanobotAmmo = 15;
	int currentActiveNanobotAmmo = 0;
	std::vector<Nanobot> nanobots;

	bool isGameWon = false;

	void ChangeWave(int waveNumber); // Use when doing wave system to change wave and accurately update displayed data, to call only once
	int GetWave();

	void ShowOverloadStack();

	std::vector<std::weak_ptr<RenderObject>> sceneMedicalTextList;
	void InitSceneMedicalText(GEOMETRY_TYPE font);
	bool AddSceneMedicalText(const std::string& text, int index = -1);
	void ClearSceneMedicalText();
};

#endif