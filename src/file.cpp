#include "file.hpp"

File_Interface::File_Interface() {}

File_Interface::~File_Interface() {}

File_Interface *File_Interface::openFile(std::string filename) {
    auto ret = new File_Interface();

    if(ret->file = std::ifstream(filename, std::ifstream::in | std::ifstream::binary)) {
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

void File_Interface::seekFile(u32 offset) {
    file.seekg(offset, std::ios_base::beg);
}

u8 File_Interface::getByte(u32 offset) {
    seekFile(offset);
    return file.get();
}

size_t File_Interface::getBuffer(u32 offset, u8 *buf, size_t len) {
    seekFile(offset);
    file.get((char *)buf, len);
    return this->file.gcount();
}