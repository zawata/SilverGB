#pragma once

#include <chrono>

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

#include "util/file.hpp"

// https://wiki.wxwidgets.org/Troubleshooting_building_wxWidgets_using_Microsoft_VC#MSVC_8.0_.282005.29_Windows_XP_Manifest_Build_Problems
#define wxUSE_NO_MANIFEST 1
#define _CRT_SECURE_NO_WARNINGS // TODO: find the specific place wxcrt.h is being included and wrap it with this definition

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#   include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
#   error "GLCanvas required"
#endif
#include "wx/glcanvas.h"

#if !wxUSE_MENUS
#   error "Menus Required"
#endif

#include "gb_core/core.hpp"
#include "util/file.hpp"
#include "util/CircularQueue.hpp"

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
    GB_Core *core = nullptr;

    RollingFrameTimeBuffer *frame_times = nullptr;
    Silver::File *rom_file = nullptr;
    Silver::File *bios_file = nullptr;

    GL::Texture2D tex;
    Shaders::Flat2D shader;

    std::chrono::high_resolution_clock::time_point prev_tp;

    ExecMode exec_mode = execMode_NORMAL;
    DisplayMode display_mode = displayMode_FIT;

public:
    Silver_ExecutionManager(wxGLCanvas *glcanvas);
    ~Silver_ExecutionManager();

    void setROM(std::string filename);
    void setBIOS(std::string filename);

    void setExecMode(ExecMode mode);
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
    void setDisplayMode(wxCommandEvent &event);
    void setScaledSize(wxCommandEvent& event);

    void startROM(wxCommandEvent& event);
    void stepExecution(wxCommandEvent& event);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void onIdle(wxIdleEvent& evt);

    void onPaint(wxPaintEvent &);

private:
    wxGLCanvas *glcanvas = nullptr;
    wxGLContext *wx_gl_ctxt = nullptr;
    Silver_ExecutionManager *exec_mngr = nullptr;

    Magnum::Platform::GLContext mgnm_ctxt;

    wxDECLARE_EVENT_TABLE();
};


// class Silver_renderPane : public wxGLCanvas {
//     wxGLContext*	m_context = nullptr;

//     GLuint
//         screen_texture,
//         screen_texture_fbo;
//     bool draw_screen_fbo;


// public:
//     Silver_renderPane(wxFrame *parent, const wxGLAttributes& canvasAttrs);
//     virtual ~Silver_renderPane();

//     void initGL();

//     void onResize(wxSizeEvent& evt);
//     void onRender(wxPaintEvent& evt);

//     GLuint getScreenTextureID();
//     GLuint getScreenTextureFBOID();

//     void setDrawScreenFBO(bool draw);

//     int getWidth();
//     int getHeight();

//     // events
//     void mouseMoved(wxMouseEvent& event);
//     void mouseDown(wxMouseEvent& event);
//     void mouseWheelMoved(wxMouseEvent& event);
//     void mouseReleased(wxMouseEvent& event);
//     void rightClick(wxMouseEvent& event);
//     void mouseLeftWindow(wxMouseEvent& event);
//     void keyPressed(wxKeyEvent& event);
//     void keyReleased(wxKeyEvent& event);

//     DECLARE_EVENT_TABLE()
// };
