#pragma once

#include <vector>
#include "Scene.h"

class SceneManager
{
public:
	static SceneManager& GetInstance();

	void PushState(Scene* pScene);
	void RequestChangeState(Scene* pScene);

	void ChangeState(Scene* pScene);
	void PopState();

	void Update(float dt);
	void Render();

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
