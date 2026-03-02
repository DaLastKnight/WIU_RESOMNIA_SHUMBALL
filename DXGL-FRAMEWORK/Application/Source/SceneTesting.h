#ifndef SCENE_BOWLING_H
#define SCENE_BOWLING_H

#include "BaseScene.h"
#include <set>

class SceneBowling : public BaseScene
{
public:

	enum GEOMETRY_TYPE : int
	{
		AXES = 0,
		GROUND,
		SKYBOX,
		LIGHT,
		GROUP,
		DEBUG_LINE,

		FONT_CASCADIA_MONO,
		// add more variables here
		FLASHLIGHT,
		BUILDING_BLOCKS,
		BUILDING_BLOCKS2,
		BALLOONS,

		BOWLING_BALL,
		BOWLING_PIN,

		TABLES_N_CHAIRS,
		Bowling_Rack,
		Bowling_Counter,

		NPC_1,

		HIT_BOX,
		BALL_HIT_BOX,

		UI_TEST,
		UI_TEST_2,

		PHYSICS_BALL,
		PHYSICS_BOX,
		TRIGGER_BOX,

		TOTAL
	};

	SceneBowling();
	~SceneBowling();

	void Enter() override;
	void Update(float dt) override;
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

	glm::vec3 savedBallWorldPos;

	//variables for bowling ball
	bool ballSpawned = false;
	bool holdingBall = false;
	bool ballThrown = false;
	const float Ball_Radius = 0.4f;
	int throwCount = 0;

	//for ALL hitboxes
	bool hit_Box = false;
	
	//for scene manager
	bool requestSceneChange = false;

	//for scoring system
	int score = 0;
	std::set<std::string> knockedDownPins;

	//For interactions
	bool nearCounter = false;
};

#endif