#include "Replay.h"
#include <fstream>

void Replay::removeActions(double time) {
    const auto check = [&] (const Action& action) -> bool {
        return action.time >= time;
    };
    actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}

void Replay::Save(const std::string& path) {
    std::ofstream file(path);
    int nActions = 0;
    file << "[HEADER]" << "\n";
    file << "Ver=" << "1.2" << "\n"; // 1.2 store time instead of frame and store xpos (to test replay in both modes but anyway save main type)
    file << "FPS=" << getFps() << "\n";
	file << "Type=" << (getType() == ReplayType::XPOS) << "\n";
    file << "[ACTIONS]" << "\n";
    for (const auto& action : actions) {
        file << action.time << ":" << action.xpos << ":" << action.holdP1 << ":" << action.holdP2 << "\n";
        ++ nActions;
    }
    std::cout << "Saved " << nActions << " actions." << std::endl;
}

Replay Replay::Load(const std::string& path) {
    Replay replay(0, ReplayType::FRAME);
    std::ifstream file(path);
    std::string line;
    Action action;
    int nActions = 0;
    bool loadActions = false;
	//bool islegacy = false;
    while(std::getline(file, line)) {
        if (line.empty()) continue;
        if(loadActions) {
            // load actions
            int posInit = 0;
            int posFound = 0;
            std::string splitted;
            std::vector<std::string> results;
            while(posFound >= 0) {
                posFound = line.find(':', posInit);
                splitted = line.substr(posInit, posFound - posInit);
                posInit = posFound + 1;
                results.push_back(splitted);
            }
            action.time = std::stod(std::string(results[0]));
            action.xpos = std::stof(std::string(results[1]));
            action.holdP1 = results[2] == "1";
            action.holdP2 = results[3] == "1";
            replay.addAction(action);
            ++ nActions;
        } else {
            // Header
			int posInit = 0;
            int posFound = 0;
            std::string splitted;
            std::vector<std::string> results;
            while(posFound >= 0) {
                posFound = line.find('=', posInit);
                splitted = line.substr(posInit, posFound - posInit);
                posInit = posFound + 1;
                results.push_back(splitted);
            }
			if(results[0] == "Type") replay.type = results[1] == "0" ? ReplayType::FRAME : ReplayType::XPOS;
			if(results[0] == "FPS") replay.fps = std::stoi(std::string(results[1]));
			//if(results[0] == "Ver") islegacy = results[1] == "1.0";
		}
        if(line == "[ACTIONS]") loadActions = true;
    }
	//if(islegacy) replay = fixLegacy(replay);
    std::cout << "loaded " << nActions << " actions." << std::endl;
	return std::move(replay);
}