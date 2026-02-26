#pragma once

#include <vector>
#include "Scene.h"

class SceneManager
{
public:
	static SceneManager& GetInstance() {
		static SceneManager instance;
		return instance;
	}

	void PushState(Scene* pScene);
	void PopState();

	void ChangeState(Scene* pScene) {
		PopState();
		PushState(pScene);
	}

	void Update(float dt);
	void Render();

	void RequestChangeState(Scene* pScene);

	Scene* m_pendingScene = nullptr;
	bool m_doChange = false;

private:
	std::vector<Scene*> m_scenes;

	//ensure an instance of this class can't be created outside of this class
	SceneManager() = default;
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
};
