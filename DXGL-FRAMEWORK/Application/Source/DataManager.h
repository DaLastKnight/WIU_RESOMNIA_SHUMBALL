#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <array>
#include <string>

class DataManager {
public:
	
    static DataManager& GetInstance() {
        static DataManager dataManager;
        return dataManager;
    }

    void SaveData();
    void LoadData();

private:

    enum GAME_TYPE {

        BOWLING,
        RHYTHM_GAME,
        WHACK_A_MOLE,
        MEDICAL_GUN,

        TOTAL_GAME_TYPE
    };

    std::string GameTypeToString(GAME_TYPE type) {
        switch (type) {
        case BOWLING: return "bowling";
        case RHYTHM_GAME: return "rhythm game";
        case WHACK_A_MOLE: return "whack a mole";
        case MEDICAL_GUN: return "medical gun";
        default: return "unknown";
        }
    }

    std::array<float, TOTAL_GAME_TYPE> highestScores;

    std::string directory = "PlayerData/";

    DataManager() {
        for (auto& score : highestScores)
            score = 0;
    }
    ~DataManager() = default;
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;
};

#endif