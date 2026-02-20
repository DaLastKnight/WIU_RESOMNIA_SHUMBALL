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
		ENV_SPHERE_MODEL,
		ENV_BLOCK_MODEL,
		ENV_STRING_MODEL,
		ENV_LIQUID_MODEL,
		ENV_LIQUID_FLAT_MODEL,

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

	void HandleKeyPress() override;

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
	
	// Medical Gun Game Scene Specifics
	int waveNumber = 1;
	int waveTimeLeft = 180;
	float waveTimeAccumulator = 0.0f;
	int maxEntitiesP = 0;
	int maxEntitiesAI = 0;
	int remainingEntitiesP = 0;
	int remainingEntitiesAI = 0;
	int overloadingState = 0;
	int maxOverload = 5;

	// Used for later during collision checking between objects
	/*void HandleCollisions();
	void Player2EnemyCollisionCheck();
	void Player2EnvironmentCollisionCheck();
	void Nanobot2EnemyCollisionCheck();*/
};

#endif