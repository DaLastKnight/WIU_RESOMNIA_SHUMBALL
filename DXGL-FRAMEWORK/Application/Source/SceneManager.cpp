#include "SceneManager.h"

SceneManager::~SceneManager()
{
	for (Scene* pScene : m_scenes)
		delete pScene;
}

void SceneManager::PushState(Scene* pScene)
{
	pScene->Enter();
	m_scenes.push_back(pScene);
}

void SceneManager::PopState()
{
	if (m_scenes.size() > 0)
	{
		m_scenes.back()->Exit();
		delete m_scenes.back();
		m_scenes.pop_back();
	}

}

void SceneManager::Update(float dt)
{
	if (m_scenes.empty())
		return;

	m_scenes.back()->Update(dt);
}

void SceneManager::Render()
{
	//draw all scenes from bottom to top
	for (Scene* pScene : m_scenes)
		pScene->Render();

	// AFTER render completes, safely switch
	if (m_doChange)
	{
		ChangeState(m_pendingScene);
		m_pendingScene = nullptr;
		m_doChange = false;
	}
}

void SceneManager::RequestChangeState(Scene* pScene)
{
	m_pendingScene = pScene;
	m_doChange = true;
}