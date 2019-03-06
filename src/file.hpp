#pragma once

#include <fstream>
#include <iostream>

#include "util/ints.hpp"

class File_Interface {
public:

    ~File_Interface();

    static File_Interface *openFile(std::string filename, bool write=false);
    static File_Interface *createFile(std::string filename);

    u32 getCRC();
    u32 getSize();

    u8 getByte(u32 offset);
    size_t getBuffer(u32 offset, u8 *buf, size_t len);

    void setByte(u32 offset, u8 data);
    void setBuffer(u32 offset, u8 *buf, size_t len);

private:
    std::fstream file;

    File_Interface();

    void seekFile_g(u32 offset);
    void seekFile_p(u32 offset);
};