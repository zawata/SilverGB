#pragma once

#include <list>
#include <string>
#include <vector>

#include "util/file.hpp"
#include "util/ints.hpp"

// Turns out win32 isn't the only OS to give me a headache 🙃
#if defined(_X11_XLIB_H_)
    #undef Bool
    #undef Status
    #undef False
    #undef True
#endif

#if defined(X_H)
    #undef None
#endif

#include <nop/serializer.h>
#include <nop/status.h>
#include <nop/structure.h>

class FileReader {
    Silver::File *file;
    u32 pos = 0;
public:
    FileReader(Silver::File *file) : file(file), pos(0) {}

    nop::Status<void> Ensure(std::size_t size) {
        if((pos + size) >= file->getSize()) {
            return nop::ErrorStatus::ReadLimitReached;
        }
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Read(std::uint8_t* byte) {
        if(pos == file->getSize()) return nop::ErrorStatus::ReadLimitReached;
        *byte = file->getByte(pos++);
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Read(void* begin, void* end) {
        size_t sz = intptr_t(end) - intptr_t(begin);
        file->getBuffer(pos, (u8*)begin, sz);
        if((pos + sz) >= file->getSize()) {
            pos = file->getSize();
            return nop::ErrorStatus::ReadLimitReached;
        }
        pos += sz;
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Skip(std::size_t padding_bytes) {
        pos += padding_bytes;
        return nop::ErrorStatus::None;
    }

    // template <typename HandleType>
    // nop::Status<HandleType> GetHandle(nop::HandleReference handle_reference) {}
};

class FileWriter {
    Silver::File *file;
    u32 pos = 0;
public:
    FileWriter(Silver::File *file) : file(file), pos(0) {}

    nop::Status<void> Prepare(std::size_t size) {
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Write(std::uint8_t byte) {
        file->setByte(pos, byte);
        pos += 1;
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Write(const void* begin, const void* end) {
        size_t sz = intptr_t(end) - intptr_t(begin);
        file->setBuffer(pos, (u8 *)begin, sz);
        pos += sz;
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Skip(std::size_t padding_bytes, std::uint8_t padding_value) {
        for(int i = 0; i < padding_bytes; i++) {
            file->setByte(pos, padding_value);
            pos += 1;
        }
        return nop::ErrorStatus::None;
    }

    // template <typename HandleType>
    // nop::Status<HandleType> GetHandle(nop::HandleReference handle_reference) {}
};

struct _Config_Section_Base {
    virtual void setDefaults() = 0;
};


struct Config_FileSettings: _Config_Section_Base {
    //abs path
    std::string recent_dir;
    //abs path + filename
    std::vector<std::string> recent_files;

    void setDefaults() {
        recent_dir = "";
        recent_files.clear();
    }

    NOP_STRUCTURE(Config_FileSettings,
            recent_dir,
            recent_files);
};

struct Config_ViewSettings: _Config_Section_Base {
    // greater than zero
    unsigned int size;

    void setDefaults() {
        size = 2;
    }

    NOP_STRUCTURE(Config_ViewSettings,
            size);
};

struct Config_EmulationSettings: _Config_Section_Base {
    //abs path + filename
    std::string bios_file;

    void setDefaults() {
        bios_file = "";
    }

    NOP_STRUCTURE(Config_EmulationSettings,
            bios_file);
};

class Config {
    static constexpr const char *filename = "Silver.cfg";
public:
    Config_FileSettings fileSettings;
    Config_ViewSettings viewSettings;
    Config_EmulationSettings emulationSettings;

    Config() {
        Load();
    }

    ~Config() {
        Save();
    }

    void Load() {
        if(Silver::File::fileExists(filename)) {
            nop::Deserializer<FileReader> deserializer(Silver::File::openFile(filename));

            deserializer.Read(&fileSettings);
            deserializer.Read(&viewSettings);
            deserializer.Read(&emulationSettings);
        } else {
            std::cout << "Loading Defaults" << std::endl;
            fileSettings.setDefaults();
            viewSettings.setDefaults();
            emulationSettings.setDefaults();
        }


    }

    void Save() {
        Silver::File *file;
        if(Silver::File::fileExists(filename)) {
            file = Silver::File::openFile(filename, true, true);
        }
        else {
            file = Silver::File::createFile(filename);
        }
        nop::Serializer<FileWriter> serializer(file);

        serializer.Write(fileSettings);
        serializer.Write(viewSettings);
        serializer.Write(emulationSettings);
    }
};