#include "file.hpp"

#include "util/crc.hpp"
#include "util/util.hpp"

File_Interface::File_Interface() {}

File_Interface::~File_Interface() {}

File_Interface *File_Interface::createFile(std::string filename) {
    if(std::ifstream(filename)) return nullptr; //don't create file if it exists

    //not casting to void throws an error about shaddowing filename?
    (void)std::ofstream(filename); //create file
    return openFile(filename, true);
}

File_Interface *File_Interface::openFile(std::string filename, bool write) {
    auto ret = new File_Interface();
    if(ret->file = std::fstream(filename,
            std::ifstream::in |
            std::ifstream::out |
            std::ifstream::binary)) {
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

u32 File_Interface::getCRC() {
    const int buf_size = 1024;
    u32 file_crc = crc.begin(), pos;
    u16 retrieved;
    u8 buf[buf_size];

    do {
        retrieved = this->getBuffer(0, buf, buf_size);

        file_crc = crc.update(file_crc, buf, retrieved);
        pos += retrieved;
    } while(retrieved == buf_size);

    return file_crc;
}

u32 File_Interface::getSize() {
    file.seekg(0, std::ios_base::end);
    return file.tellg();
}

u8 File_Interface::getByte(u32 offset) {
    seekFile_g(offset);
    return file.get();
}

size_t File_Interface::getBuffer(u32 offset, u8 *buf, size_t len) {
    seekFile_g(offset);
    file.get((char *)buf, len + 1);
    return this->file.gcount();
}

void File_Interface::setByte(u32 offset, u8 data) {
    seekFile_p(offset);
    file.put(data);
    file.flush();
}

void File_Interface::setBuffer(u32 offset, u8 *buf, size_t len) {
    seekFile_p(offset);
    file.write((char *)buf, len);
    file.flush();
}

/**
 * Private
 */
void File_Interface::seekFile_g(u32 offset) {
    file.seekg(offset, std::ios_base::beg);
}

void File_Interface::seekFile_p(u32 offset) {
    file.seekp(offset, std::ios_base::beg);
}