#pragma once

#include <algorithm>
#include <cstdio>
#include <string>

#include <nowide/iostream.hpp>

#include <wx/version.h>
#include <wx/glcanvas.h>
#include <wx/string.h>

#include "cfg/cfg.hpp"

inline std::string wx_to_utf8(wxString wx) {
#if wxCHECK_VERSION(3,1,5)
    return wx.utf8_string();
#elif wxCHECK_VERSION(3,1,1)
    return wx.ToStdString(wxConvUTF8);
#else
    wxScopedCharBuffer buf(wx.utf8_str());
    return std::string(buf.data(), buf.length());
#endif
}

inline wxGLCanvas *init_wxGLCanvas(wxWindow *parent) {
    //TODO: I'm not sure about how required most of these options are.
#if wxCHECK_VERSION(3,1,0)
    wxGLAttributes attributes;
    attributes.PlatformDefaults()
              .BufferSize(24)
              .MinRGBA(8, 8, 8, 0)
              .Depth(24)
              .Stencil(0)
              .DoubleBuffer()
              .EndList();
    return new wxGLCanvas{
        parent,
        attributes
    };
#else
    int attrib_list[] = {
        WX_GL_BUFFER_SIZE, 24,
        WX_GL_MIN_RED, 8,
        WX_GL_MIN_GREEN, 8,
        WX_GL_MIN_BLUE, 8,
        WX_GL_MIN_ALPHA, 0,
        WX_GL_DEPTH_SIZE, 24,
        WX_GL_STENCIL_SIZE, 0,
        WX_GL_DOUBLEBUFFER,
        0 // terminate the list
    };

    return new wxGLCanvas{
        parent,
        wxID_ANY,
        attrib_list
    };
#endif
}

inline void add_to_MRU(std::shared_ptr<Config> config, std::string path) {
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