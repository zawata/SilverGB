#include "cfg.hpp"

Configuration *Configuration::loadConfigFile(std::string filepath) {
    File_Interface *f;
    if(!(f=File_Interface::openFile(filepath))) {
        std::cerr << "Error opening config file" << std::endl;
        return nullptr;
    }

    Configuration c;
    size_t retreived = f->getBuffer(0, (u8 *)&c, sizeof(Configuration));
    if(retreived != sizeof(c)) {
        std::cerr << "Config File not big enough." << std::endl;
        return nullptr;
    }

    return new Configuration(c); // Hooray implicit copy constructors
}

Configuration::Configuration() {}

Configuration::~Configuration() {}

bool Configuration::saveConfigFile(std::string filepath) {
    File_Interface *f;
    if(!(f=File_Interface::openFile(filepath))) {
        std::cerr << "Error opening config file" << std::endl;
        return false;
    }

    f->setBuffer(0,(u8 *)this, sizeof(Configuration));
}

