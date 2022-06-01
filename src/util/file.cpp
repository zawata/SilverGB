#include <nowide/iostream.hpp>

#include "util/file.hpp"

#include "util/crc.hpp"
#include "util/util.hpp"

namespace Silver {

FileIterator::FileIterator(File *f, u32 o): f(f), o(o) {}
FileIterator::~FileIterator() {}

FileIterator &FileIterator::operator ++()                           { o++; return *this; }
FileIterator  FileIterator::operator ++(int)                        { return FileIterator(f, o++); }
FileIterator  FileIterator::operator +(u32 i) const                 { return FileIterator(f, o + i); }
FileIterator &FileIterator::operator --()                           { o--; return *this; }
FileIterator  FileIterator::operator --(int)                        { return FileIterator(f, o--); }
FileIterator  FileIterator::operator -(u32 i) const                 { return FileIterator(f, o - i); }
u8            FileIterator::operator *()                            { return f->getByte(o); }
bool          FileIterator::operator ==(FileIterator const&b) const { return f == b.f && o == b.o; }
bool          FileIterator::operator !=(FileIterator const&b) const { return !(*this == b); }

File::File(std::string const& filename) :
filename(filename) {}

File::~File() {
    file.close();
}

File *File::createFile(std::string filename) {
    if(nowide::ifstream(filename)) return nullptr; //don't create file if it exists

    //cast to void to avoid shadowing the function argument
    (void)nowide::ofstream(filename); //create file
    return openFile(filename, true);
}

File *File::openFile(std::string filename, bool write, bool trunc) {
    auto ret = new File(filename);

    std::ios::openmode mode = nowide::ifstream::binary;

    if(write) {
        mode |= nowide::ifstream::out;

        if(trunc) {
            mode |= nowide::ifstream::trunc;
        }
    } else {
        mode |= nowide::ifstream::in;
    }

    if((ret->file = nowide::fstream(filename, mode))) {
        ret->file.exceptions(nowide::ifstream::failbit | nowide::ifstream::badbit);
        return ret;
    } else {
        delete ret;
        return nullptr;
    }
}

bool File::fileExists(std::string filename) {
    return (bool)nowide::ifstream(filename);
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

size_t File::getBuffer(u32 offset, void *buf, size_t len) {
    seekFile_g(offset);
    file.read((char *)buf, len);
    return this->file.gcount();
}

void File::setByte(u32 offset, u8 data) {
    seekFile_p(offset);
    file.put(data);
    file.flush();
}

void File::setBuffer(u32 offset, void *buf, size_t len) {
    seekFile_p(offset);
    file.write((char *)buf, len);
    file.flush();
}

FileIterator File::begin() {
    return FileIterator(this, 0);
}

FileIterator File::end() {
    return FileIterator(this, getSize());
}

std::string File::getFilename() {
    return filename;
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