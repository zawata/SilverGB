#pragma once

#include <algorithm>
#include <cstdio>
#include <string>

#include <nowide/iostream.hpp>

#include <wx/version.h>
#include <wx/string.h>

#include "cfg.hpp"

std::string wx_to_utf8(wxString wx) {
#if wxCHECK_VERSION(3,1,5)
    return wx.utf8_string();
#else
    return wx.ToStdString(wxConvUTF8);
#endif
}

inline void add_to_MRU(Config *config, std::string path) {
    auto& recent_files = config->fileSettings.recent_files;

    recent_files.erase(std::remove_if(recent_files.begin(), recent_files.end(), [path](auto const& file) {
        return file == path;
    }), recent_files.end());

    while(recent_files.size() >= 10) {
        recent_files.pop_back();
    }

    for(auto const& f : recent_files) {
        nowide::cout << f << std::endl;
    }

    recent_files.insert(recent_files.begin(), 1, path);
}