#pragma once

#include "file.hpp"

class Configuration {
public:
    static Configuration *loadConfigFile(std::string filepath);

    Configuration();
    ~Configuration();

    bool saveConfigFile(std::string filepath);

    struct __config_data {
        bool bin_enabled = false;
        char bin_file[256] = { 0 };
    } config_data;
};