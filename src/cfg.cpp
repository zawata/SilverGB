#include "cfg.hpp"

#include <cstring>

Configuration *Configuration::loadConfigFile(std::string filepath) {
    File_Interface *f;
    if(!(f=File_Interface::openFile(filepath))) {
        std::cerr << "Error opening config file" << std::endl;
        return nullptr;
    }

    Configuration c;

    //I don't remember if the class would be implicitly zeroed...
    // but it doesn't hurt to keep this here for now
    memset(&(c.config_data), 0, sizeof(__config_data));

    size_t retreived = f->getBuffer(0, (u8 *)&(c.config_data), sizeof(__config_data));
    if(retreived != sizeof(__config_data)) {
        std::cerr << "Warning: Config File not big enough." << std::endl;
    }

    return new Configuration(c); // Hooray implicit copy constructors
}

Configuration::Configuration() {}

Configuration::~Configuration() {}

bool Configuration::saveConfigFile(std::string filepath) {
    File_Interface *f;
    if(f=File_Interface::createFile(filepath)) {
        f->setBuffer(0,(u8 *)&config_data, sizeof(__config_data));
        delete f;
        return true;
    } else if(f=File_Interface::openFile(filepath, true)) {
        f->setBuffer(0,(u8 *)&config_data, sizeof(__config_data));
        delete f;
        return true;
    }
    std::cerr << "Error saving config file" << std::endl;
    return false;
}

