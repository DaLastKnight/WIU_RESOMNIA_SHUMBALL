#include "DataManager.h"
#include "Console.h"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;
using nlohmann::ordered_json;


void DataManager::SaveData(DATA dataType, int value) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		data[dataType] = value;
		return;
	}
	Error(" DataManager::UpdateData(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

void DataManager::UpdateData(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		auto& current = data[dataType];
		auto& buffer = data_buffer[dataType];
		auto& highScoreChanged = data_highScoreChanged[dataType];
		auto& localHighScoreChanged = data_localHighScoreChanged[dataType];
		auto& stored = data_stored[dataType];

		highScoreChanged = stored < current;
		if (dataType == MEDICAL_TIMETAKEN) {
			if (stored == 0)
				highScoreChanged = true;
			else 
				highScoreChanged = stored > current;
		}

		if (buffer != 0) {
			localHighScoreChanged = buffer < current;
			if (dataType == MEDICAL_TIMETAKEN) {
				localHighScoreChanged = buffer > current;
			}
		}
		else {
			localHighScoreChanged = highScoreChanged;
		}

		if ((buffer == 0 && highScoreChanged) || localHighScoreChanged)
			buffer = current;

		return;
	}
	Error(" DataManager::SaveData(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");

}

void DataManager::SaveAllDataToFile() {
	ordered_json savedData = json::object();

	for (int i = 0; i < TOTAL_DATA; i++) {
		auto& buffer = data_buffer[i];

		if (buffer != 0) {
			savedData[DataToString(static_cast<DATA>(i))] = buffer;
		}
		else {
			savedData[DataToString(static_cast<DATA>(i))] = data_stored[i];
		}
	}

	SaveFile(savedData, "PlayerData.json");
}

void DataManager::LoadData() {
	ordered_json loadedData = LoadFile("PlayerData.json");

	for (int i = 0; i < TOTAL_DATA; i++) {
		data_stored[i] = loadedData[DataToString(static_cast<DATA>(i))].get<int>();
	}

	Print("successfully loaded player data", 2);
}

bool DataManager::HighScoreChanged(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		return data_highScoreChanged[dataType];
	}
	Error(" DataManager::HighScoreChanged(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

bool DataManager::LocalHighScoreChanged(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		return data_localHighScoreChanged[dataType];
	}
	Error(" DataManager::HighScoreChanged(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

int DataManager::GetPrevHghScoreData(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		return data_stored[dataType];
	}
	Error(" DataManager::GetPrevHghScoreData(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

int DataManager::GetThisHighScoreData(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		if (data_highScoreChanged[dataType])
			return data_buffer[dataType];
		else
			return data_stored[dataType];
	}
	Error(" DataManager::GetPrevHghScoreData(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

int DataManager::GetCurrentData(DATA dataType) {
	if (dataType < TOTAL_DATA && dataType >= 0) {
		return data[dataType];
	}
	Error(" DataManager::GetPrevHghScoreData(): invalid data, no such data represented as \"" + std::to_string(dataType) + "\" exist");
}

std::string DataManager::DataToString(DATA dataType) {
	switch (dataType) {
	case MEDICAL_TIMETAKEN: return "MEDICAL_TIMETAKEN";
	case WHACK_SCORE: return "WHACK_SCORE";
	case RHYTHM_SCORE_DIFF0: return "RHYTHM_SCORE_DIFF0";
	case RHYTHM_SCORE_DIFF1: return "RHYTHM_SCORE_DIFF1";
	case COLLAB_SCORE: return "COLLAB_SCORE";
	default: return "unknown";
	}
}

nlohmann::ordered_json DataManager::LoadFile(std::string filePath) {
	std::string fullPath = directory + filePath;
	std::ifstream inFile(fullPath);
	if (!inFile.is_open()) {
		std::cerr << "Failed to load file at \"" << fullPath << "\"" << std::endl;
		return ordered_json();
	}
	ordered_json loadedFile;
	inFile >> loadedFile;
	inFile.close();

	Print("loaded file at \"" + fullPath + "\"", 2);

	return loadedFile;
}

void DataManager::SaveFile(nlohmann::ordered_json json, std::string filePath) {
	std::string fullPath = directory + filePath;
	std::ofstream outFile(fullPath);
	if (!outFile.is_open()) {
		std::cerr << "Failed to save file at \"" << fullPath << "\"" << std::endl;
		return;
	}

	outFile << json.dump(4);
	outFile.close();

	Print("saved to file at \"" + fullPath + "\"", 2);
}
