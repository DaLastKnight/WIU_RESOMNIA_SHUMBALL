#ifndef SCENE_BOWLING_H
#define SCENE_BOWLING_H

#include "BaseScene.h"


enum class GEOMETRY_TYPE : int
{
	AXES = 0,
	GROUND,
	SKYBOX,
	LIGHT,
	GROUP,

	FONT_CASCADIA_MONO,

	// add more variables here
	FLASHLIGHT,
	Bowling_Pin,
	Bowling_Ball,

	UI_TEST,
	UI_TEST_2,

	TOTAL
};

class SceneBowling : public BaseScene
{
public:

	SceneBowling();
	~SceneBowling();

	void Init() override;
	void Update(double dt) override;
	void Render() override;
	void Exit() override;

private:

	enum SFX_TYPE {
		GOOFY_AHH_ASRIEL_STAR_SOUND,

		TOTAL_SFX
	};

	float FontSpacing(GEOMETRY_TYPE font) override {
		switch (font) {
		case GEOMETRY_TYPE::FONT_CASCADIA_MONO: return 0.375f;
		default: return 1;
		}
	}

	void HandleKeyPress() override;




	
};

#endif