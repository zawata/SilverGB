#pragma once

#include <string>
#include <vector>

#include "util/file.hpp"
#include "util/types/primitives.hpp"

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

#if defined(_OBJC_OBJC_H_)
#undef Nil
#endif

#include <nop/serializer.h>
#include <nop/status.h>
#include <nop/structure.h>

class FileReader {
    Silver::File *file;
    u32           pos = 0;

public:
    FileReader(Silver::File *file) :
        file(file), pos(0) { }

    nop::Status<void> Ensure(std::size_t size) {
        if((pos + size) >= file->getSize()) {
            return nop::ErrorStatus::ReadLimitReached;
        }
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Read(std::uint8_t *byte) {
        if(pos == file->getSize()) {
            return nop::ErrorStatus::ReadLimitReached;
        }
        *byte = file->getByte(pos++);
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Read(void *begin, void *end) {
        size_t sz = intptr_t(end) - intptr_t(begin);
        file->getBuffer(pos, (u8 *)begin, sz);
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
    u32           pos = 0;

public:
    FileWriter(Silver::File *file) :
        file(file), pos(0) { }

    nop::Status<void> Prepare(std::size_t size) { return nop::ErrorStatus::None; }

    nop::Status<void> Write(std::uint8_t byte) {
        file->setByte(pos, byte);
        pos += 1;
        return nop::ErrorStatus::None;
    }

    nop::Status<void> Write(const void *begin, const void *end) {
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
    // abs path
    std::string              recent_dir;
    // abs path + filename
    std::vector<std::string> recent_files;

    void                     setDefaults() override {
        recent_dir = "";
        recent_files.clear();
    }

    NOP_STRUCTURE(Config_FileSettings, recent_dir, recent_files);
};

struct Config_DisplaySettings: _Config_Section_Base {
    // greater than zero
    unsigned int size;

    void         setDefaults() override { size = 2; }

    NOP_STRUCTURE(Config_DisplaySettings, size);
};

struct Config_EmulationSettings: _Config_Section_Base {
    // abs path + filename
    std::string bios_file;
    bool        enable_frame_skip = false;
    int         frame_skip        = 0;

    void        setDefaults() override {
        bios_file         = "";
        enable_frame_skip = false;
        frame_skip        = 0;
    }

    NOP_STRUCTURE(Config_EmulationSettings, bios_file, enable_frame_skip, frame_skip);
};

struct Config_AudioSettings: _Config_Section_Base {
    bool enable = true;
    int  volume = 100;

    void setDefaults() override {
        enable = true;
        volume = 100;
    }

    NOP_STRUCTURE(Config_AudioSettings, enable, volume);
};

struct Config_InputSettings: _Config_Section_Base {
    // abs path + filename
    std::string layout_file;

    void        setDefaults() override { layout_file = ""; }

    NOP_STRUCTURE(Config_InputSettings, layout_file);
};

struct Config_DebugSettings: _Config_Section_Base {
    bool enable_debug_mode = false;

    void setDefaults() override { enable_debug_mode = false; }

    NOP_STRUCTURE(Config_DebugSettings, enable_debug_mode);
};

class Config {
    static constexpr const char *filename = "Silver.cfg";

public:
    Config_FileSettings      file;
    Config_DisplaySettings   display;
    Config_EmulationSettings emu;
    Config_AudioSettings     audio;
    Config_InputSettings     input;
    Config_DebugSettings     debug;

    Config() { Load(); }

    ~Config() { Save(); }

    void Load() {
        if(Silver::File::fileExists(filename)) {
            nop::Deserializer<FileReader> deserializer(Silver::File::openFile(filename));

            deserializer.Read(&file);
            deserializer.Read(&display);
            deserializer.Read(&emu);
            deserializer.Read(&audio);
            deserializer.Read(&input);
            deserializer.Read(&debug);
        } else {
            file.setDefaults();
            display.setDefaults();
            emu.setDefaults();
            audio.setDefaults();
            input.setDefaults();
            debug.setDefaults();
        }
    }

    void Save() const {
        Silver::File *settingsFile;
        if(Silver::File::fileExists(filename)) {
            settingsFile = Silver::File::openFile(filename, true, true);
        } else {
            settingsFile = Silver::File::createFile(filename);
        }
        nop::Serializer<FileWriter> serializer(settingsFile);

        serializer.Write(file);
        serializer.Write(display);
        serializer.Write(emu);
        serializer.Write(audio);
        serializer.Write(input);
        serializer.Write(debug);
    }
};