#pragma once

#include "file.hpp"

class Configuration {
public:
    static Configuration *loadConfigFile(std::string filepath);

    Configuration();
    ~Configuration();

    bool saveConfigFile(std::string filepath);

    struct __config_data {
        __config_data() = default;

        bool bin_enabled;
        char bin_file[256];
    } config_data;
};