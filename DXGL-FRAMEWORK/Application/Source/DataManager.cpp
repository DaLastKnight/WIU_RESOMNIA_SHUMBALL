#include "DataManager.h"
#include "Application.h"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

using App = Application;

using nlohmann::json;
using nlohmann::ordered_json;


void DataManager::SaveData() {
	std::string fullPath = directory + "PlayerData.json";
	std::ofstream outFile(fullPath);
	if (!outFile.is_open()) {
		std::cerr << "Failed to save file at \"" << fullPath << "\"" << std::endl;
		return;
	}
	ordered_json savedData = json::object();
	
	ordered_json savedScores = json::object();
	{
		for (int i = 0; i < TOTAL_GAME_TYPE; i++) {
			savedScores[GameTypeToString(static_cast<GAME_TYPE>(i))] = highestScores[i];
		}
	}
	savedData["highestScores"] = savedScores;


	outFile << savedData.dump(4); // saves json content into the file with indent of 4 space for each bracket
	outFile.close();

	App::print("saved map to file at \"" + fullPath + "\"", 2);
}


void DataManager::LoadData() {
	std::string fullPath = directory + "PlayerData.json";
	std::ifstream inFile(fullPath);
	if (!inFile.is_open()) {
		std::cerr << "Failed to load file at \"" << fullPath << "\"" << std::endl;
		return;
	}
	ordered_json loadedData;
	inFile >> loadedData;
	inFile.close();

	ordered_json loadedScores = loadedData["highestScores"];
	for (int i = 0; i < TOTAL_GAME_TYPE; i++) {
		highestScores[i] = loadedScores[GameTypeToString(static_cast<GAME_TYPE>(i))];
	}

	App::print("loaded map from file at \"" + fullPath + "\"", 2);
}
