#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <array>
#include <string>

#include <nlohmann/json.hpp>

class DataManager {
public:

    enum DATA {
        MEDICAL_TIMETAKEN,
        WACK_SCORE,
        RHYTHM_SCORE_DIFF0,
        RHYTHM_SCORE_DIFF1,
        COLLAB_SCORE,

        TOTAL_DATA
    };
	
    static DataManager& GetInstance() {
        static DataManager dataManager;
        return dataManager;
    }

    void SaveData(DATA dataType, int value);
    void UpdateData(DATA dataType);
    void SaveAllDataToFile();
    void LoadData();
    bool HighScoreChanged(DATA dataType);
    bool LocalHighScoreChanged(DATA dataType);

    int GetPrevHghScoreData(DATA dataType);
    int GetThisHighScoreData(DATA dataType);

    std::string DataToString(DATA dataType);

private:

    nlohmann::ordered_json LoadFile(std::string filePath);
    void SaveFile(nlohmann::ordered_json json, std::string filePath);

    std::string directory = "PlayerData/";

    // current data / for end result of 1 program run
    std::array<int, TOTAL_DATA> data;

    // if data to be stored is better than data stored previously
    std::array<bool, TOTAL_DATA> data_highScoreChanged;

    // if data to be stored is better than data to be stored previously
    std::array<bool, TOTAL_DATA> data_localHighScoreChanged;

    // data to be (checked with) store
    std::array<int, TOTAL_DATA> data_buffer;

    // data stored previously
    std::array<int, TOTAL_DATA> data_stored;

    DataManager() {
        for (int i = 0; i < TOTAL_DATA; i++) {
            data_buffer[i] = data_stored[i] = -1;
        }
    }
    ~DataManager() = default;
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;
};

#endif