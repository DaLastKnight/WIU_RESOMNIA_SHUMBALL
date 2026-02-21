#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

/*
	How to use GameStateManager:
	GameStateManager is a singleton class, where it manages the state of the game
	between scenes (instead of having a GameStateManager per scene). Simply use 
	if-statements or switch statements to enable certain mechanics in the game
	based on the game state shared between scenes. Here's the functions to use:

	SetGameState: Sets the current game state
	GetCurrentGameState: Returns the current game state
	GetPreviousGameState: Returns the previous game state
	CheckSameState: In-built function to return whether current game state == argument
*/

enum class GAME_STATE
{
	GAMEPLAY,
	CUTSCENE,
	MENU,
	PAUSED,
};

class GameStateManager
{
public:

	static GameStateManager& GetInstance()
	{
		static GameStateManager gameStateManager;
		return gameStateManager;
	}

	void SetGameState(GAME_STATE newState);
	GAME_STATE GetCurrentGameState() const;
	GAME_STATE GetPreviousGameState() const;
	bool CheckSameState(GAME_STATE stateChecked) const;

private:
	GAME_STATE currentState = GAME_STATE::GAMEPLAY;
	GAME_STATE previousState = GAME_STATE::GAMEPLAY;

	GameStateManager() = default;
	~GameStateManager() = default;
	GameStateManager(const GameStateManager&) = delete;
	GameStateManager& operator=(const GameStateManager&) = delete;
};

#endif