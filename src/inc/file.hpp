#include <fstream>

#include "util.hpp"

#ifndef FILE_H
#define FILE_H

class File_Interface {
public:
    
    ~File_Interface();

    static File_Interface *openFile(std::string filename);

    void seekFile(u32 offset);

    u8 getByte(u32 offset);
    size_t getBuffer(u32 offset, u8 *buf, size_t len);

private:
    File_Interface();

    std::ifstream file;
};

#endif