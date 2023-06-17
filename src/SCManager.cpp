#include "SCManager.h"
#include <filesystem>
#include <fstream>

std::filesystem::path getSaveFilePath() {
    const auto path = CCFileUtils::sharedFileUtils()->getWritablePath();
    return std::filesystem::path(path) / "SCModManager.dat";
}

inline std::pair<std::string, std::string> splitKey(const std::string str, char split) {
    const auto n = str.find(split);
    return { str.substr(0, n), str.substr(n + 1) };
}

void createSaveFile(){
    const auto path = getSaveFilePath();
    std::ofstream file(path);
}

std::vector<std::string> getSaveFile() {
    const auto path = getSaveFilePath();
    if(!std::filesystem::exists(path)) createSaveFile();
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while(std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

void saveCfgFile(std::vector<std::string> file) {
    const auto path = getSaveFilePath();
    std::ofstream savefile(path);
    size_t pos = 0;
    while(pos < file.size()) {
        savefile << file[pos] << "\n";
        pos++;
    }
}

void SCManager::setSCModVariable(std::string key, bool value) {
    std::string tostr = value ? "true" : "false";
    SCManager::setSCModString(key, tostr);
}

void SCManager::setSCModString(std::string key, std::string value) {
    std::vector<std::string> saveFile = getSaveFile();
    size_t pos = 0;
    bool isfound = false;
    while(pos < saveFile.size()) {
        const auto [orikey, valuex] = splitKey(saveFile[pos], '=');
        if(key == orikey) {
            isfound = true;
            saveFile[pos] = key + "=" + value;
        }
        pos++;
    }
    if(!isfound) saveFile.push_back(key + "=" + value);
    saveCfgFile(saveFile);
}

bool SCManager::getSCModVariable(std::string key) {
    std::vector<std::string> saveFile = getSaveFile();
    size_t pos = 0;
    while (pos < saveFile.size()) {
        const auto [orikey, value] = splitKey(saveFile[pos], '=');
        if (key == orikey) {
            return value == "true";
        }
        pos++;
    }
    return false;
}

std::string SCManager::getSCModString(std::string key) {
    std::vector<std::string> saveFile = getSaveFile();
    size_t pos = 0;
    while (pos < saveFile.size()) {
        const auto [orikey, value] = splitKey(saveFile[pos], '=');
        if (key == orikey) {
            return value;
        }
        pos++;
    }
    return "";
}