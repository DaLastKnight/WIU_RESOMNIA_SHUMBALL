#ifndef SCENE_DEMO_H
#define SCENE_DEMO_H

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

	UI_TEST,
	UI_TEST_2,

	TOTAL
};

class SceneDemo : public BaseScene
{
public:

	SceneDemo();
	~SceneDemo();

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