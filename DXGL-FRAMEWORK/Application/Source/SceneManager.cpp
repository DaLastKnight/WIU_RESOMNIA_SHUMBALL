#include "SceneManager.h"

SceneManager& SceneManager::GetInstance()
{
	static SceneManager instance;
	return instance;
}

SceneManager::~SceneManager()
{
	for (Scene* pScene : m_scenes)
		delete pScene;
}

void SceneManager::PushState(Scene* pScene)
{
	size_t size = m_scenes.size();
	if (size > 0)
		m_scenes[size - 1]->Pause();

	pScene->Enter();
	m_scenes.push_back(pScene);
}

void SceneManager::ChangeState(Scene* pScene)
{
	size_t size = m_scenes.size();
	if (size > 0)
	{
		m_scenes[size - 1]->Exit();
		delete m_scenes[size - 1];
		m_scenes.pop_back();
	}

	pScene->Enter();
	m_scenes.push_back(pScene);
}

void SceneManager::PopState()
{
	size_t size = m_scenes.size();
	if (size > 0)
	{
		m_scenes[size - 1]->Exit();
		delete m_scenes[size - 1];
		m_scenes.pop_back();
	}

	if (m_scenes.size() > 0)
	{
		m_scenes[m_scenes.size() - 1]->Resume();
	}
}

//void SceneManager::Update(float dt)
//{
//	size_t size = m_scenes.size();
//	if (size == 0)
//		return; //no scenes to update
//
//	m_scenes[size - 1]->Update(dt);
//}

void SceneManager::Update(float dt)
{
	if (m_scenes.empty())
		return;

	m_scenes.back()->Update(dt);

	// AFTER update completes, safely switch
	if (m_doChange)
	{
		ChangeState(m_pendingScene);
		m_pendingScene = nullptr;
		m_doChange = false;
	}
}

void SceneManager::Render()
{
	//draw all scenes from bottom to top
	for (Scene* pScene : m_scenes)
		pScene->Render();
}

void SceneManager::RequestChangeState(Scene* pScene)
{
	m_pendingScene = pScene;
	m_doChange = true;
}