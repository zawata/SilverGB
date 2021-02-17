#include <iterator>
#include <vector>

#include "util/file.hpp"

#include "util/crc.hpp"
#include "util/util.hpp"

namespace Silver {

File::File(std::string filename) :
filename(filename) {}

File::~File() {}

File *File::createFile(std::string filename) {
    if(std::ifstream(filename)) return nullptr; //don't create file if it exists

    //not casting to void throws an error about shaddowing filename?
    (void)std::ofstream(filename); //create file
    return openFile(filename, true);
}

File *File::openFile(std::string filename, bool write, bool trunc) {
    auto ret = new File(filename);
    if((ret->file = std::fstream(filename,
            std::ifstream::in |
            ((write) ? std::ifstream::out : 0) |
            ((trunc) ? std::ifstream::trunc : 0) |
            std::ifstream::binary))) {
        ret->file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

bool File::fileExists(std::string filename) {
    return (bool)std::ifstream(filename);
}

u32 File::getCRC() {
    const int buf_size = 1024;
    u32 file_crc = 0, pos = 0;
    u16 retrieved;
    u8 buf[buf_size];

    crc::begin();
    do {
        retrieved = this->getBuffer(pos, buf, buf_size);

        file_crc = crc::update(file_crc, buf, retrieved);
        pos += retrieved;
    } while(retrieved == buf_size);

    return file_crc;
}

u32 File::getSize() {
    file.seekg(0, std::ios_base::end);
    return (u32)file.tellg();
}

u8 File::getByte(u32 offset) {
    seekFile_g(offset);
    return (u8)file.get();
}

size_t File::getBuffer(u32 offset, u8 *buf, size_t len) {
    seekFile_g(offset);
    file.read((char *)buf, len);
    return this->file.gcount();
}

void File::setByte(u32 offset, u8 data) {
    seekFile_p(offset);
    file.put(data);
    file.flush();
}

void File::setBuffer(u32 offset, u8 *buf, size_t len) {
    seekFile_p(offset);
    file.write((char *)buf, len);
    file.flush();
}

std::string File::getFilename() {
    return filename;
}

void File::toVector(std::vector<u8> &vec) {
    u32 size = this->getSize();
    vec.clear();
    vec.resize(size);

    //std::copy(std::istream_iterator<u8>(file), std::istream_iterator<u8>(), std::back_inserter(vec));
    getBuffer(0, vec.data(), size);
}

void File::fromVector(std::vector<u8> const& vec) {
    seekFile_p(0);
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<char>(file));
}

/**
 * Private
 */
void File::seekFile_g(u32 offset) {
    file.seekg(offset, std::ios_base::beg);
}

void File::seekFile_p(u32 offset) {
    file.seekp(offset, std::ios_base::beg);
}

} // namespace Silver