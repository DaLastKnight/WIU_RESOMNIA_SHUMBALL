#ifndef SCENE_WHACK_H
#define SCENE_WHACK_H

#include "BaseScene.h"
#include <list>
#include <random>

#include "Virus.h"

class SceneWhack : public BaseScene
{
public:
	enum GAMESTATE
	{
		GAMEPLAY = 0,
		FREEROAM,
		DIALOGUE,
	};
	
	enum CATEGORY
	{
		CATEGORY_1 = 0x0001, // Player
		CATEGORY_2 = 0x0002, // Environment objects (grounds, walls, etc)
		CATEGORY_3 = 0x0004, // In-game physics objects (trigger box, etc)
		CATEGORY_4 = 0x0008, // Balls
		CATEGORY_5 = 0x0010, // Viruses
		CATEGORY_6 = 0x0020, // Portals
	};


	class BallTimer
	{
	public:
		
		float timer;
		std::weak_ptr<RenderObject> ball;

		BallTimer(float timer, std::weak_ptr<RenderObject> ball) :
			timer(timer), ball(ball) {}
	};

	class Portal
	{
	public:
		glm::vec3 portalBasePosition;
		std::weak_ptr<RenderObject> portal;

		Portal(glm::vec3 basePosition, std::weak_ptr<RenderObject> portal) :
			portalBasePosition(basePosition), portal(portal) {}
	};
	
	enum GEOMETRY_TYPE : int
	{
		AXES = 0,

		// Scene stuff
		LIGHT,
		GROUP,
		DEBUG_LINE,
		FONT_CASCADIA_MONO,
		SKYBOX,
		
		// World Models
		GROUND,
		PLATFORM,
		WALL,
		TERMINAL,
		TRIGGER_BOX,
		ANTIVIRUS_BALL,
		VIRUS_1,
		VIRUS_2,
		VIRUS_3,
		PORTAL_EVIL,
		PORTAL_GOOD,
		CROSSHAIR,

		// View Models
		

		// Screen Models
		WHITE,
		GAUGE,

		

		TOTAL
	};

	SceneWhack();
	~SceneWhack();

	void Init() override;
	void Update(double dt) override;
	void Render() override;
	void Exit() override;

private:

	// Pre-made scene variables (don't touch unless assets)
	bool dirtyWorldList = false;

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

	// NOW my actual variables I'm using

	// Game members
	bool gamePaused = false;
	bool canStartGame = true;
	bool inGame = false;
	float gameTimer = 60;
	int gameScore = 0;
	double elapsed = 0;

	// Player UI members
	glm::vec3 gaugeMarkerPosition;
	bool isGaugeActive;
	
	// Virus members
	std::vector<Virus> virusList;
	double elapsedVirusInterval = 3;
	int FindVirusIndex(const std::shared_ptr<RenderObject>& obj)
	{
		for (int k = 0; k < (int)virusList.size(); ++k)
		{
			auto virusObj = virusList[k].virus.lock();
			if (virusObj && virusObj.get() == obj.get())
				return k;
		}
		return -1;
	}

	// Anti-virus ball members
	std::vector<BallTimer> ballTimeList;
	int FindTimerIndex(const std::shared_ptr<RenderObject>& obj)
	{
		for (int k = 0; k < (int)ballTimeList.size(); ++k)
		{
			auto timerObj = ballTimeList[k].ball.lock();
			if (timerObj && timerObj.get() == obj.get())
				return k;
		}
		return -1;
	}

	// Portal members (so many...)
	glm::vec3 selectedPortalPosition;
	Portal* portalGround = nullptr;
	std::vector<Portal> portalEvilList;
	std::vector<Portal> portalGoodList;
	std::array<std::vector<glm::vec3>, 2> portalEvilMapList;
	std::array<std::vector<glm::vec3>, 2> portalGoodMapList;
	std::vector<glm::vec3> portalEvilPositionsList;
	std::vector<glm::vec3> portalGoodPositionsList;
	int FindEvilPortalIndex(const std::shared_ptr<RenderObject>& obj, bool isGood = false)
	{	
		for (int k = 0; k < (int)portalEvilList.size(); ++k)
		{
			auto portalObj = portalEvilList[k].portal.lock();
			if (portalObj && portalObj.get() == obj.get())
				return k;
		}
		return -1;
	}
	int FindGoodPortalIndex(const std::shared_ptr<RenderObject>& obj)
	{
		for (int k = 0; k < (int)portalGoodList.size(); ++k)
		{
			auto portalObj = portalGoodList[k].portal.lock();
			if (portalObj && portalObj.get() == obj.get())
				return k;
		}
		return -1;
	}
	void InitPortalMaps();
	void ChangePortalMap();

	// Cutscene members
	void StartCutscene();
	bool hasFinishedTutorial = false;

	
};

#endif