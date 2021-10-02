#pragma once

#include <cassert>
#include <iterator>
#include <vector>

#include <nowide/fstream.hpp>

#include "util/ints.hpp"

namespace Silver {

class File {
public:

    ~File();

    static File *openFile(std::string filename, bool write=false, bool trunc=false);
    static File *createFile(std::string filename);

    static bool fileExists(std::string);

    template<typename T>
    void toVector(std::vector<T> &vec);

    template<typename T>
    void fromVector(std::vector<T> const& vec);

    u32 getCRC();
    u32 getSize();

    u8 getByte(u32 offset);
    size_t getBuffer(u32 offset, u8 *buf, size_t len);

    void setByte(u32 offset, u8 data);
    void setBuffer(u32 offset, u8 *buf, size_t len);

    std::string getFilename();

private:
    nowide::fstream file;
    std::string filename;

    File(std::string filename);

    void seekFile_g(u32 offset);
    void seekFile_p(u32 offset);
};

template<typename T>
void File::toVector(std::vector<T> &vec) {
    static_assert(std::is_pod<T>::value);

    u32 size = this->getSize();

    assert(size % sizeof(T) == 0);

    vec.clear();
    vec.resize(size);

    std::copy(std::istream_iterator<T>(file), std::istream_iterator<T>(), std::back_inserter(vec));
    // getBuffer(0, vec.data(), size);
}

template<typename T>
void File::fromVector(std::vector<T> const& vec) {
    seekFile_p(0);
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(file));
}

} // namespace Silver