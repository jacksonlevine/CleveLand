#ifndef SETTINGS_H
#define SETTINGS_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

struct Setting {
    std::string name;
    std::string value;
};

struct Settings {
    void loadOrSaveSettings(std::vector<Setting> &inout);
    void saveSettings(std::vector<Setting>& in);
};

#endif