#pragma once

#include <cassert>
#include <iterator>
#include <nowide/fstream.hpp>
#include <nowide/iostream.hpp>
#include <vector>

#include "util/ints.hpp"

namespace Silver {

class File;

class FileIterator {
    friend File;
    File *f;
    u32   o;
    FileIterator(File *f, u32 o);

public:
    ~FileIterator();

    FileIterator &operator++();
    FileIterator  operator++(int);
    FileIterator  operator+(u32 i) const;

    FileIterator &operator--();
    FileIterator  operator--(int);
    FileIterator  operator-(u32 i) const;

    u8 operator*();

    bool operator==(FileIterator const &b) const;
    bool operator!=(FileIterator const &b) const;
};

class File {
    friend FileIterator;

public:
    using iterator = FileIterator;

    ~File();

    static File *openFile(std::string filename, bool write = false, bool trunc = false);
    static File *createFile(std::string filename);

    static bool fileExists(std::string);

    template<typename T>
    void toVector(std::vector<T> &vec);

    template<typename T>
    void fromVector(std::vector<T> const &vec);

    u32 getCRC();
    u32 getSize();

    u8     getByte(u32 offset);
    size_t getBuffer(u32 offset, void *buf, size_t len);

    void setByte(u32 offset, u8 data);
    void setBuffer(u32 offset, void *buf, size_t len);

    FileIterator begin();
    FileIterator end();

    std::string getFilename();

private:
    nowide::fstream file;
    std::string     filename;

    explicit File(std::string const &filename);

    void seekFile_g(u32 offset);
    void seekFile_p(u32 offset);
};

template<typename T>
void File::toVector(std::vector<T> &vec) {
    static_assert(std::is_pod<T>::value, "");

    u32 size = this->getSize();

    assert(size % sizeof(T) == 0);

    vec.clear();
    vec.resize(size);

    getBuffer(0, vec.data(), size);
}

template<typename T>
void File::fromVector(std::vector<T> const &vec) {
    static_assert(std::is_pod<T>::value, "");

    setBuffer(0, (void *)vec.data(), vec.size() * sizeof(T));

    // not sure why this doesn't work? the saved file is 4-bytes too small
    // std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(file));
}

} // namespace Silver