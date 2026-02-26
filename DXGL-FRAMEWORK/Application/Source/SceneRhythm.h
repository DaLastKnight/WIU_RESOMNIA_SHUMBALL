#ifndef SCENE_RHYTHM_H
#define SCENE_RHYTHM_H

#include "BaseScene.h"

#include <GL/glew.h>
#include "SRhythmRaycast.h"

#include "Console.h"


class SceneRhythm : public BaseScene
{
public:

	SceneRhythm();
	~SceneRhythm();

	void Init() override;
	void Update(double dt) override;
	void Render() override;
	void Exit() override;

private:

	bool dirtyWorldList = false;

	enum STATE {
		LOAD_IN,
		START_INTERMISSION,
		INTERMISSION,
		END_INTERMISSION,
		START_GAME,
		GAME,
		START_RESULT,
		RESULT,
		END_RESULT,
		EXIT,

		TOTAL_STATE
	};

	STATE currentState;

	float dynamicStateTimer = 0;

	enum GEOMETRY_TYPE : int
	{
		AXES = 0,
		GROUND,
		SKYBOX,
		LIGHT,
		GROUP,

		FONT_CASCADIA_MONO,

		BLACK,

		RT_BASE_UI,
		RT_PANEL_BASE,
		RT_DISC,
		RT_OPU,
		RT_OPU_BASE,
		RT_UPLOAD_BTN,
		RT_UPLOAD_BTN_BG,
		RT_LOSSLESS_BTN,
		RT_LOSSLESS_BTN_BG,
		RT_COMPRESSED_BTN,
		RT_COMPRESSED_BTN_BG,
		RT_NEXT_BTN,
		RT_NEXT_BTN_BG,
		RT_RETRY_BTN,
		RT_RETRY_BTN_BG,
		RT_PROGRESSION,
		RT_PROGRESS,
		RT_PROGRESS_INDICATOR,
		
		RHYTHM_BASE,
		RHYTHM_BEAT,
		RHYTHM_HIT_POINT,
		RHYTHM_TAP_NOTE,
		RHYTHM_HOLD_NOTE,
		RHYTHM_HOLD_NOTE_BODY,

		VFX_TAP_NOTE,
		VFX_HOLD_NOTE,
		VFX_HOLD_NOTE_BODY,

		TRIGGER_BOX,
		INVISIBLE_WALL,

		TOTAL
	};

	enum SFX_TYPE {
		SFX_CLICK,
		SFX_FAIL,
		SFX_TADA,
		SFX_SWOOSH,
		SFX_TICK,

		SFX_LANE0,
		SFX_LANE1,
		SFX_LANE2,
		SFX_LANE3,

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

	std::map<std::string, std::weak_ptr<RenderObject>> uiPointsOfInterest;
	std::map<std::string, std::weak_ptr<RenderObject>> inGamePointsOfInterest;
	std::vector<std::weak_ptr<RenderObject>> VFXList;
	std::map<std::string, std::weak_ptr<RenderObject>> triggerList;

	PhysicsRaycast physicsRaycast;

	// texture
	std::array<GLuint, 2> baseUITexture;

	// game info
	int difficulty = 1;
	glm::vec3 globalSurroundingColor;
	float atmosphereTargetingDensestRange;

	// text
	bool allowActivatePacketLoss = false;
	bool allowActivateUploadSuccessful = false;
	glm::vec3 situationTextColor = glm::vec3(1);

	// input info
	bool lmb_pressed;
	bool lmb_down;
	bool lmb_released;
	bool rmb_pressed;
	bool rmb_down;
	bool rmb_released;

	// sfx
	float resultSFXTimer = 0;

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
	
};

#endif