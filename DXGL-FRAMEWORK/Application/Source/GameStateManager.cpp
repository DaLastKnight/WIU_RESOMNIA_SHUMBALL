#include "DialogueManager.h"

#include "Console.h"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include "GameStateManager.h"

using nlohmann::json;
using nlohmann::ordered_json;

void GameStateManager::SetGameState(GAME_STATE newState)
{
	previousState = currentState;
	currentState = newState;
}

GAME_STATE GameStateManager::GetCurrentGameState() const
{
	return currentState;
}

GAME_STATE GameStateManager::GetPreviousGameState() const
{
	return previousState;
}

bool GameStateManager::CheckSameState(GAME_STATE stateChecked) const
{
	return currentState == stateChecked;
}
