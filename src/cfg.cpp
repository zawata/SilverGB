#include "cfg.hpp"

#include <cstring>
#include <iostream>

#include <glib/gkeyfile.h>

Configuration *Configuration::loadConfigFile(std::string filepath) {
    GKeyFile *cfg_file = g_key_file_new();
    GError *error = nullptr;

    if(!g_key_file_load_from_file(cfg_file, filepath.c_str(), G_KEY_FILE_NONE, &error)) {
        switch(error->code) {
        case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
        case G_KEY_FILE_ERROR_PARSE:
            std::cerr << "File not valid" << std::endl;
            break;
        case G_KEY_FILE_ERROR_NOT_FOUND:
            std::cerr << "File not found" << std::endl;
            break;
        }
        return nullptr;
    } else {
        return new Configuration(cfg_file, filepath);
    }
}

Configuration *Configuration::newConfigFile(std::string filepath) {
    GKeyFile *cfg_file = g_key_file_new();

    return new Configuration(cfg_file, filepath);
}


Configuration::Configuration(GKeyFile *file, std::string filepath) :
Configuration_Fields(file),
cfg_file(file),
default_filepath(filepath) {}
Configuration::~Configuration() {
    g_key_file_free(cfg_file);
}

bool Configuration::saveConfigFile() {
    GError *error = nullptr;
    if(!g_key_file_save_to_file(cfg_file, this->default_filepath.c_str(), &error)) {
        switch(error->code) {
        case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
        case G_KEY_FILE_ERROR_PARSE:
            std::cout << "File not valid" << std::endl;
            break;
        case G_KEY_FILE_ERROR_NOT_FOUND:
            std::cout << "File not found" << std::endl;
            break;
        }
        return false;
    }
    return true;
}

bool Configuration::saveConfigFile(std::string filepath) {
    GError *error = nullptr;
    if(!g_key_file_save_to_file(cfg_file, filepath.c_str(), &error)) {
        switch(error->code) {
        case G_KEY_FILE_ERROR_UNKNOWN_ENCODING:
        case G_KEY_FILE_ERROR_PARSE:
            std::cout << "File not valid" << std::endl;
            break;
        case G_KEY_FILE_ERROR_NOT_FOUND:
            std::cout << "File not found" << std::endl;
            break;
        }
        return false;
    }
    return true;
}

