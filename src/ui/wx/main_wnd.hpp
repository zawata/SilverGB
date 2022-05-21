#pragma once

#include <chrono>
#include <memory>

#include <Corrade/Containers/Reference.h>

#include <Magnum/DimensionTraits.h>

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/BufferImage.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Version.h>

#include <Magnum/Math/Color.h>

#include <Magnum/Platform/GLContext.h>

#include <Magnum/Shaders/Flat.h>


// https://wiki.wxwidgets.org/Troubleshooting_building_wxWidgets_using_Microsoft_VC#MSVC_8.0_.282005.29_Windows_XP_Manifest_Build_Problems
#define wxUSE_NO_MANIFEST 1
#define _CRT_SECURE_NO_WARNINGS // TODO: find the specific place wxcrt.h is being included and wrap it with this definition

#include "wx/wx.h"

#if !wxUSE_GLCANVAS
#   error "GLCanvas required"
#endif
#include "wx/glcanvas.h"

#if !wxUSE_MENUS
#   error "Menus Required"
#endif

#include "gb_core/core.hpp"
#include "gb_core/defs.hpp"
#include "util/circular_queue.hpp"
#include "util/file.hpp"

#include "cfg/cfg.hpp"

/**
 * Class Forward Declarations
 */
class SilverGBApp;
class Silver_coreTimer;
class Silver_mainFrame;
class Silver_renderPane;

struct RollingFrameTimeBuffer : CircularQueue<float> {
    RollingFrameTimeBuffer() : CircularQueue<float>(60) {}

    void insert(float time) {
        if(size() == 60) dequeue();

        enqueue(time);
    }

    float average() {
        if(size() != 60) return -1.0f;

        float *ptr = start;
        float avg = 0;
        while(ptr != end) avg += ( *ptr++ * (1.0f/60.0f) );

        return avg;
    }
};

using namespace Magnum;
class FullscreenTextureShader;

class Silver_ExecutionManager {
public:
    enum ExecMode {
        execMode_NORMAL,
        execMode_TICK_CLOCK,
        execMode_TICK_INSTR,
        execMode_TICK_FRAME
    };

    enum DisplayMode {
        displayMode_STRETCH,
        displayMode_FIT,
        displayMode_CENTER
    };

private:
    wxGLCanvas *glcanvas = nullptr;
    Silver::Core *core = nullptr;

    RollingFrameTimeBuffer *frame_times = nullptr;
    Silver::File *rom_file = nullptr;
    Silver::File *bios_file = nullptr;

    GL::Texture2D *tex = nullptr;
    Shaders::Flat2D *shader = nullptr;

    std::chrono::high_resolution_clock::time_point prev_tp;

    ExecMode exec_mode = execMode_NORMAL;
    DisplayMode display_mode = displayMode_FIT;
    gb_device_t dev = device_GB;

public:
    Silver_ExecutionManager(wxGLCanvas *glcanvas);
    ~Silver_ExecutionManager();

    void setROM(std::string filename);
    void setBIOS(std::string filename);

    void setExecMode(ExecMode mode);
    void setDevice(gb_device_t dev);
    void setDisplayMode(DisplayMode mode);

    void startCore();
    void endCore();
    bool execLoop();
    void execStep();

    void onGLDraw();
};

class SilverGBApp : public wxApp {
public:
    SilverGBApp() {}
    ~SilverGBApp() {}
    bool OnInit() override;
};

class Silver_mainFrame : public wxFrame {
public:
    Silver_mainFrame(const wxString& title);

    void loadROM(wxCommandEvent &event);
    void loadRecent(wxCommandEvent& event);
    void setBIOS(wxCommandEvent &event);

    void setExecMode(wxCommandEvent &event);
    void setDevice(wxCommandEvent& event);
    void setDisplayMode(wxCommandEvent &event);
    void setScaledSize(wxCommandEvent& event);

    void startROM(wxCommandEvent& event);
    void stepExecution(wxCommandEvent& event);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void onIdle(wxIdleEvent& evt);
    void onResize(wxSizeEvent &);
    void onPaint(wxPaintEvent &);

private:
    std::shared_ptr<Config> config;
    wxGLCanvas *glcanvas = nullptr;
    wxGLContext *wx_gl_ctxt = nullptr;
    Silver_ExecutionManager *exec_mngr = nullptr;

    Magnum::Platform::GLContext mgnm_ctxt;

    wxDECLARE_EVENT_TABLE();
};
