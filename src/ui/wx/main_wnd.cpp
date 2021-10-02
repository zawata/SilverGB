#include "main_wnd.hpp"

#include "Magnum/GL/GL.h"
#include "Magnum/GL/Mesh.h"
#include "Magnum/Math/Tags.h"
#include "Magnum/Shaders/Flat.h"
#include "gb_core/defs.hpp"

#include "wx/string.h"
#include "wx/version.h"

#include <chrono>
#include <codecvt>
#include <locale>

#include <nowide/iostream.hpp>

#define check_gl_error() _check_gl_error(__LINE__)
void _check_gl_error(u32 line) {
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        nowide::cerr << "GL" << line << ": " << as_hex(err) << std::endl;
    }
}

std::string wx_to_utf8(wxString wx) {
#if wxCHECK_VERSION(3,1,5)
    return wx.utf8_string();
#else
    return wx.ToStdString(wxConvUTF8);
#endif
}

/*********************************************************************
 * SilverGB Application
 */

#define wxID_next() (wxID_HIGHEST + __LINE__)

enum {
    id_File_LoadROM = wxID_next(),
    id_File_LoadBIOS = wxID_next(),
    id_File_Quit = wxID_EXIT,

    id_Emulation_ExecMode_Normal = wxID_next(),
    id_Emulation_ExecMode_StepClk = wxID_next(),
    id_Emulation_ExecMode_StepInstr = wxID_next(),
    id_Emulation_ExecMode_StepFrame = wxID_next(),
    id_Emulation_step = wxID_next(),

    id_View_DisplayMode_Stretch = wxID_next(),
    id_View_DisplayMode_Fit = wxID_next(),
    id_View_DisplayMode_Center = wxID_next(),
    id_View_Size_1x1 = wxID_next(),
    id_View_Size_2x2 = wxID_next(),
    id_View_Size_3x3 = wxID_next(),
    id_View_Size_4x4 = wxID_next(),
    id_View_Size_5x5 = wxID_next(),
    id_View_Size_6x6 = wxID_next(),
    id_View_Size_7x7 = wxID_next(),
    id_View_Size_8x8 = wxID_next(),

    id_Help_About = wxID_ABOUT,
};

wxBEGIN_EVENT_TABLE(Silver_mainFrame, wxFrame)
    EVT_MENU(id_File_LoadROM, Silver_mainFrame::loadROM)
    EVT_MENU(id_File_LoadBIOS, Silver_mainFrame::loadBIOS)

    EVT_MENU(id_File_Quit,    Silver_mainFrame::OnQuit)

    EVT_MENU(id_Emulation_ExecMode_Normal, Silver_mainFrame::setExecMode)
    EVT_MENU(id_Emulation_ExecMode_StepClk, Silver_mainFrame::setExecMode)
    EVT_MENU(id_Emulation_ExecMode_StepInstr, Silver_mainFrame::setExecMode)
    EVT_MENU(id_Emulation_ExecMode_StepFrame, Silver_mainFrame::setExecMode)

    EVT_MENU(id_Emulation_step, Silver_mainFrame::stepExecution)

    EVT_MENU(id_View_DisplayMode_Stretch, Silver_mainFrame::setDisplayMode)
    EVT_MENU(id_View_DisplayMode_Fit, Silver_mainFrame::setDisplayMode)
    EVT_MENU(id_View_DisplayMode_Center, Silver_mainFrame::setDisplayMode)

    EVT_MENU(id_View_Size_1x1, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_2x2, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_3x3, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_4x4, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_5x5, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_6x6, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_7x7, Silver_mainFrame::setScaledSize)
    EVT_MENU(id_View_Size_8x8, Silver_mainFrame::setScaledSize)

    EVT_MENU(id_Help_About,   Silver_mainFrame::OnAbout)

    EVT_IDLE(Silver_mainFrame::onIdle)
wxEND_EVENT_TABLE()

//DO NOT TOUCH
//msvc/clang/wxwidgets seems confused about which particular entry function it/they should be using
//these settings coupled with those in cmakelists work for now.
wxIMPLEMENT_APP(SilverGBApp);

bool SilverGBApp::OnInit() {
    if ( !wxApp::OnInit() )
        return false;

    nowide::cout << "🎉" << std::endl;

    Silver_mainFrame* frame = new Silver_mainFrame("SilverGB");

    frame->Show(true);

    return true;
}

/*********************************************************************
 * SilverGB Frame
 */
Silver_mainFrame::Silver_mainFrame(const wxString& title) : wxFrame{nullptr, wxID_ANY, title}, mgnm_ctxt{Magnum::NoCreate} {
    wxBoxSizer* bSizer;
    bSizer = new wxBoxSizer{wxVERTICAL};

    wxGLAttributes attributes;
    attributes.PlatformDefaults()
              .BufferSize(24)
              .MinRGBA(8, 8, 8, 0)
              .Depth(24)
              .Stencil(0)
              .DoubleBuffer()
              .EndList();
    glcanvas = new wxGLCanvas{this, attributes, wxID_ANY, wxDefaultPosition};

    wx_gl_ctxt = new wxGLContext{glcanvas};
    glcanvas->SetCurrent(*wx_gl_ctxt);
    mgnm_ctxt.create();

    bSizer->Add(glcanvas, 1, wxALL|wxEXPAND);
    SetSizer(bSizer);
    Layout();
    bSizer->Fit(this);

    glcanvas->Connect(wxEVT_PAINT, wxPaintEventHandler(Silver_mainFrame::onPaint), nullptr, this);
    glcanvas->Refresh(); //force a paint event to init GL

    exec_mngr = new Silver_ExecutionManager(glcanvas);

    wxMenuBar *menuBar = new wxMenuBar(); {
        wxMenu *fileMenu = new wxMenu(); {
            fileMenu->Append(id_File_LoadROM,  "Load Rom\tCtrl-F");
            fileMenu->Append(id_File_LoadBIOS, "Load BIOS");
            fileMenu->Append(id_File_Quit, "Exit\tCtrl-W");
        }
        menuBar->Append(fileMenu, "File");

        wxMenu *emuMenu = new wxMenu(); {
            wxMenu *execModeMenu = new wxMenu(); {
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_Normal,    "Normal");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepClk,   "Step Clock Cycle");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepInstr, "Step Instruction");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepFrame, "Step Frame");

                execModeMenu->Check(id_Emulation_ExecMode_Normal, true );
            }
            emuMenu->AppendSubMenu(execModeMenu, "Execution Mode", "Set Execution Mode");

            emuMenu->Append(id_Emulation_step, "Step\tCtrl-T", "Step According to Execution Mode");
        }
        menuBar->Append(emuMenu, "Emulation");

        wxMenu *viewMenu = new wxMenu(); {
            wxMenu *dispModeMenu = new wxMenu(); {
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Fit,     "Fit");
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Stretch, "Stretch");
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Center,  "Fixed");

                dispModeMenu->Check(id_View_DisplayMode_Fit, true );
            }
            viewMenu->AppendSubMenu(dispModeMenu, "Display Mode");

            wxMenu *sizeMenu = new wxMenu(); {
                sizeMenu->AppendRadioItem(id_View_Size_1x1, "x1");
                sizeMenu->AppendRadioItem(id_View_Size_2x2, "x2");
                sizeMenu->AppendRadioItem(id_View_Size_3x3, "x3");
                sizeMenu->AppendRadioItem(id_View_Size_4x4, "x4");
                sizeMenu->AppendRadioItem(id_View_Size_5x5, "x5");
                sizeMenu->AppendRadioItem(id_View_Size_6x6, "x6");
                sizeMenu->AppendRadioItem(id_View_Size_7x7, "x7");
                sizeMenu->AppendRadioItem(id_View_Size_8x8, "x8");

                sizeMenu->Check(id_View_Size_2x2, true );
            }
            viewMenu->AppendSubMenu(sizeMenu, "Size");
        }
        menuBar->Append(viewMenu, "View");

        wxMenu *helpMenu = new wxMenu(); {
            helpMenu->Append(id_Help_About, "About\tF1", "Show about dialog");
        }
        menuBar->Append(helpMenu, "Help");
    }

    SetMenuBar(menuBar);

    // 2 is the default scaling factor
    SetClientSize(wxSize(GB_Core::native_width, GB_Core::native_height) * 2);
    SetMinClientSize(wxSize(GB_Core::native_width, GB_Core::native_height));\
    Centre(wxBOTH);
}

void Silver_mainFrame::loadROM(wxCommandEvent& event) {
    wxFileDialog openFileDialog(
            this,
            wxT("Open ROM file"),
            "",
            "",
            wxT("Gameboy ROM files (*.gb;*.bin)|*.gb;*.bin"),
            wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() != wxID_OK)
        return;

    exec_mngr->setROM(wx_to_utf8(openFileDialog.GetPath()));
    exec_mngr->startCore();
}

void Silver_mainFrame::loadBIOS(wxCommandEvent& event) {
    wxFileDialog openFileDialog(
            this,
            "Open Gameboy BIOS",
            "",
            "",
            "Gameboy BIOS|DMG_ROM.bin",
            wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() != wxID_OK)
        return;

    exec_mngr->setBIOS(wx_to_utf8(openFileDialog.GetPath()));
}

void Silver_mainFrame::startROM(wxCommandEvent& event) {  }

void Silver_mainFrame::setExecMode(wxCommandEvent& event) {
    switch(event.GetId()) {
    case id_Emulation_ExecMode_Normal:
        exec_mngr->setExecMode(Silver_ExecutionManager::execMode_NORMAL);
        return;
    case id_Emulation_ExecMode_StepClk:
        exec_mngr->setExecMode(Silver_ExecutionManager::execMode_TICK_CLOCK);
        return;
    case id_Emulation_ExecMode_StepInstr:
        exec_mngr->setExecMode(Silver_ExecutionManager::execMode_TICK_INSTR);
        return;
    case id_Emulation_ExecMode_StepFrame:
        exec_mngr->setExecMode(Silver_ExecutionManager::execMode_TICK_FRAME);
        return;
    }
    assert(false);
}

void Silver_mainFrame::setDisplayMode(wxCommandEvent& event) {
    switch(event.GetId()) {
    case id_View_DisplayMode_Stretch:
        exec_mngr->setDisplayMode(Silver_ExecutionManager::displayMode_STRETCH);
    case id_View_DisplayMode_Fit:
        exec_mngr->setDisplayMode(Silver_ExecutionManager::displayMode_FIT);
    case id_View_DisplayMode_Center:
        exec_mngr->setDisplayMode(Silver_ExecutionManager::displayMode_CENTER);
    }
    assert(false);
}

void Silver_mainFrame::setScaledSize(wxCommandEvent& event) {
    int scaling_factor = 1;
    switch(event.GetId()) {
    case id_View_Size_1x1:
        scaling_factor = 1;
        break;
    case id_View_Size_2x2:
        scaling_factor = 2;
        break;
    case id_View_Size_3x3:
        scaling_factor = 3;
        break;
    case id_View_Size_4x4:
        scaling_factor = 4;
        break;
    case id_View_Size_5x5:
        scaling_factor = 5;
        break;
    case id_View_Size_6x6:
        scaling_factor = 6;
        break;
    case id_View_Size_7x7:
        scaling_factor = 7;
        break;
    case id_View_Size_8x8:
        scaling_factor = 8;
        break;
    }

    SetClientSize(wxSize(GB_Core::native_width,GB_Core::native_height) * scaling_factor);
}

void Silver_mainFrame::stepExecution(wxCommandEvent& event) {
    exec_mngr->execStep();
}

void Silver_mainFrame::OnQuit(wxCommandEvent& event) {
    exec_mngr->endCore();
    Close(true);
}

void Silver_mainFrame::OnAbout(wxCommandEvent& event) {
    wxMessageBox(
            "SilverGB\nA Naive Gameboy Emulator By John Alden",
            "About",
            wxOK | wxICON_INFORMATION,
            this);
}

void Silver_mainFrame::onIdle(wxIdleEvent& evt) {
    if(exec_mngr) evt.RequestMore(exec_mngr->execLoop());
}

void Silver_mainFrame::onPaint(wxPaintEvent&) {
    if(exec_mngr) exec_mngr->onGLDraw();
}

/*********************************************************************
 * Core Execution Manager
 */

Silver_ExecutionManager::Silver_ExecutionManager(wxGLCanvas *glcanvas) :
glcanvas(glcanvas),
core(nullptr),
frame_times(new RollingFrameTimeBuffer()),
rom_file(nullptr),
bios_file(nullptr),
prev_tp(std::chrono::high_resolution_clock::now()){}

Silver_ExecutionManager::~Silver_ExecutionManager() {
    endCore();
}

void Silver_ExecutionManager::setROM(std::string filename)  { rom_file = Silver::File::openFile(filename); }
void Silver_ExecutionManager::setBIOS(std::string filename) { bios_file = Silver::File::openFile(filename); }
void Silver_ExecutionManager::setExecMode(ExecMode mode) { std::wcout << "set execMode: " << (this->exec_mode = mode) << std::endl; }
void Silver_ExecutionManager::setDisplayMode(DisplayMode mode) { this->display_mode = mode; }

void Silver_ExecutionManager::startCore() {
    if(core) endCore(); //TODO: should this restart or return?

    if(!rom_file) {
        wxMessageBox("No ROM File Loaded.", "No ROM File Loaded.");
        return;
    }

    core = new GB_Core(rom_file, bios_file);
}

bool Silver_ExecutionManager::execLoop() {
    using namespace std::chrono;

    // 16.6ms = 1/60
    #define DEFAULT_DURATION microseconds(16666);

    if(core == nullptr || exec_mode != execMode_NORMAL) return false;

    auto curr_tp = high_resolution_clock::now();
    auto curr_duration = duration_cast<microseconds>(curr_tp - prev_tp);

    duration<long long, std::micro> tgt_duration;
    float avg_framerate = frame_times->average();
    if(avg_framerate > 0) {
        if(curr_duration > seconds(1)) {
            // execution is now unpaused, but our rolling average should be restarted
            frame_times->clear();

            tgt_duration = DEFAULT_DURATION;
        } else {
            // execution has not been paused, calculate a target duration value that will correct our current framerate towards 60
            tgt_duration = microseconds(16666 + (16666 - (long long)(avg_framerate * 1000)));
        }
    }
    else {
        // execution was paused recently, run at default duration until we can calculate framerate
        tgt_duration = DEFAULT_DURATION;
    }

    if(curr_duration > tgt_duration) {
        float frame_time = curr_duration.count() / 1000.0f;
        frame_times->insert(frame_time);
        prev_tp = curr_tp;

        core->tick_frame();

        using namespace Magnum;
        using namespace Magnum::GL;
        std::vector<u8> screen_data;
        screen_data.resize(GB_S_P_SZ);
        memcpy(screen_data.data(), core->getScreenBuffer(), GB_S_P_SZ);

        tex.setSubImage(
                0,
                {0,0},
                GL::BufferImage2D(
                        GL::PixelFormat::RGB,
                        GL::PixelType::UnsignedByte,
                        {GB_S_W, GB_S_H},
                        std::move(screen_data),
                        GL::BufferUsage::StaticDraw));

        glcanvas->Refresh();

    }
    return true;
}

void Silver_ExecutionManager::execStep() {
    if(core == nullptr || exec_mode == execMode_NORMAL) return;

    switch(exec_mode) {
        case execMode_TICK_CLOCK:
            core->tick_once();
            break;
        case execMode_TICK_INSTR:
            core->tick_instr();
            break;
        case execMode_TICK_FRAME:
            core->tick_frame();
            break;
        default:
            assert(false);
    }

    using namespace Magnum;
    using namespace Magnum::GL;
    Containers::ArrayView<const void> screen_view{core->getScreenBuffer(), GB_S_P_SZ};
    tex.setSubImage(
            0,
            {0,0},
            GL::BufferImage2D(
                    GL::PixelFormat::RGB,
                    GL::PixelType::UnsignedByte,
                    {GB_S_W, GB_S_H},
                    screen_view,
                    GL::BufferUsage::StaticDraw));

    glcanvas->Refresh();
}

void Silver_ExecutionManager::onGLDraw() {
    static bool initialized = false;
    if(!initialized) {
        initialized = true;

        shader = Shaders::Flat2D(Shaders::Flat3D::Flag::Textured);
        tex = GL::Texture2D();

        tex.setWrapping(GL::SamplerWrapping::ClampToEdge)
        .setMagnificationFilter(GL::SamplerFilter::Linear)
        .setMinificationFilter(GL::SamplerFilter::Linear)
        .setStorage(1, GL::TextureFormat::RGB8, Vector2i(GB_S_W, GB_S_H));
    }

    GL::defaultFramebuffer
        .clear(GL::FramebufferClear::Color)
        .setViewport({{0,0},{glcanvas->GetSize().x, glcanvas->GetSize().y}});

    struct Vertex {
        Vector2 position;
        Vector2 textureCoordinates;
    };

    /*******************************
     * Drawing this out because I keep forgeting it

    Vertex Coordinate System:

        ( 1,-1)          ( 1, 1)
               +--------+
               |        |
               |        |
               |        |
               +--------+
        (-1,-1)          (-1, 1)


    Texture Coordinate System:

        (0,1)          (1,1)
             +--------+
             |        |
             |        |
             |        |
             +--------+
        (0,0)          (1, 0)


    *******************************/

    Vertex vertices[4]{
        {{-1.0, -1.0},{0.0,0.0}},
        {{-1.0 , 1.0},{1.0,0.0}},
        {{ 1.0, -1.0},{0.0,1.0}},
        {{ 1.0,  1.0},{1.0,1.0}},
    };

    using namespace Magnum;
    GL::Buffer v_buffer;
    v_buffer.setData(vertices, GL::BufferUsage::StaticDraw);

    GL::Mesh mesh;
    mesh.setPrimitive(GL::MeshPrimitive::TriangleStrip).setCount(4)
        .addVertexBuffer(v_buffer, 0,
        Shaders::Flat2D::Position{},
        Shaders::Flat2D::TextureCoordinates{});

    shader
        .bindTexture(tex)
        .draw(std::move(mesh));

    glcanvas->SwapBuffers();
}

void Silver_ExecutionManager::endCore() {
    if(!core) return;
    delete core;
    core = nullptr;
}

/*********************************************************************
 * SilverGB RenderPane
 */

std::pair<wxPoint, wxPoint> get_screen_area(wxSize const& win_bounds) {
    //auto screen sizing and placement code
    wxPoint img_bottom_left;
    wxPoint img_top_right;

    float scaling_factor = ::fmin(win_bounds.x/GB_S_W, win_bounds.y/GB_S_H);

    //calculate img size
    img_top_right.x = scaling_factor * GB_S_W;
    img_top_right.y = scaling_factor * GB_S_H;

    img_bottom_left.x = (win_bounds.x - img_top_right.x) / 2;
    img_bottom_left.y = (win_bounds.y - img_top_right.y) / 2;

    img_top_right.x += img_bottom_left.x;
    img_top_right.y += img_bottom_left.y;

    return std::make_pair(img_bottom_left, img_top_right);
}