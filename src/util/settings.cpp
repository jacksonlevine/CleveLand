#include "settings.h"

void Settings::loadOrSaveSettings(std::vector<Setting> &inout) {
    if(!std::filesystem::exists("prefs.save")) {
        std::ofstream file("prefs.save");
        if(file.is_open()) {
            for(Setting &setting : inout) {
                file << setting.name << " " << setting.value << "\n";
            }
        }
        file.close();
    } else {
        std::ifstream file("prefs.save");
        std::string line;
        if(file.is_open()) {
            while(std::getline(file, line)) {
                std::istringstream linestream(line);

                std::string word;
                std::vector<std::string> words;

                while(linestream >> word) {
                    words.push_back(word);
                }

                for(Setting &setting : inout) {
                    if(setting.name == words.at(0)) {
                        setting.value = words.at(1);
                    }
                }

            }
        }
        file.close();
    }
}

void Settings::saveSettings(std::vector<Setting>& in) {
    std::ofstream file("prefs.save", std::ios::trunc);
        if(file.is_open()) {
            for(Setting &setting : in) {
                file << setting.name << " " << setting.value << "\n";
            }
        }
    file.close();
}