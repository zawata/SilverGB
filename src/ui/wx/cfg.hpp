#pragma once

#include "nop/base/serializer.h"
#include "util/file.hpp"
#include "util/ints.hpp"

#include <list>
#include <string>
#include <vcruntime.h>
#include <vector>

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

    template <typename HandleType>
    nop::Status<HandleType> GetHandle(nop::HandleReference handle_reference) {}
};

struct Config_FileSettings {
    //abs path + filename
    std::string bios_file;
    //abs path
    std::string recent_dir;
    //abs path + filename
    std::array<std::string, 10> recent_files;

    NOP_STRUCTURE(Config_FileSettings,
            bios_file,
            recent_dir,
            recent_files);
};

struct Config_ViewSettings {

};

struct Config_EmulationSettings {

};

class Config {

public:
    Config_FileSettings fileSettings;
    Config_ViewSettings viewSettings;
    Config_EmulationSettings emulationSettings;

    void Load() {
        if(Silver::File::fileExists("SilverGB.cfg")) {
            nop::Deserializer<FileReader> deserializer(Silver::File::openFile("SilverGB.cfg"));

            deserializer.Read(&fileSettings);
            // deserializer.Read(viewSettings);
            // deserializer.Read(emulationSettings);
        }


    }

    void Save() {
        Silver::File *file;
        if(Silver::File::fileExists("SilverGB.cfg")) {
            file = Silver::File::openFile("SilverGB.cfg");
        }
        else {
            file = Silver::File::createFile("SilverGB.cfg");
        }

        nop::Serializer<FileWriter> serializer(file);

        serializer.Write(fileSettings);
        // serializer.Write(viewSettings);
        // serializer.Write(emulationSettings);
    }
};