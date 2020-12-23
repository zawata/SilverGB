#pragma once

#include <util/file.hpp>
#include <cfg_fields.hpp>

#include <glib-2.0/glib.h>

class Configuration : public Configuration_Fields {
public:
    static Configuration *loadConfigFile(std::string filepath);
    static Configuration *newConfigFile(std::string filepath);

    ~Configuration();

    bool saveConfigFile();
    bool saveConfigFile(std::string filepath);
private:
    Configuration(GKeyFile *cfg_file, std::string filepath);

    GKeyFile *cfg_file;
    std::string default_filepath;
};