#ifndef SCENE_RHYTHM_H
#define SCENE_RHYTHM_H

#include "BaseScene.h"


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
		INTERMISSION,
		IN_GAME,
		RESULT,
		EXIT,

		TOTAL_STATE
	};

	STATE currentState;

	float loadInTimer = 0;

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

		RHYTHM_BASE,

		TRIGGER_BOX,

		TOTAL
	};

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
	
};

#endif