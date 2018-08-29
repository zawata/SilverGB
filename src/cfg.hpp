#pragma once

#include "file.hpp"

struct Configuration {
    static Configuration *loadConfigFile(std::string filepath);

    Configuration();
    ~Configuration();

    bool saveConfigFile(std::string filepath);

    //Config data
    bool bin_enabled = false;
    const char bin_file[256] = {0};
};