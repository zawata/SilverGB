#pragma once

#include <fstream>
#include <iostream>

#include "util/ints.hpp"

namespace Silver {

class File {
public:

    ~File();

    static File *openFile(std::string filename, bool write=false);
    static File *createFile(std::string filename);

    static bool fileExists(std::string);

    u32 getCRC();
    u32 getSize();

    u8 getByte(u32 offset);
    size_t getBuffer(u32 offset, u8 *buf, size_t len);

    void setByte(u32 offset, u8 data);
    void setBuffer(u32 offset, u8 *buf, size_t len);

    std::string getFilename();

private:
    std::fstream file;
    std::string filename;

    File(std::string filename);

    void seekFile_g(u32 offset);
    void seekFile_p(u32 offset);
};

} // namespace Silver