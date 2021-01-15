#include "gui.hpp"

#include "GL/gl3w.h"
#include "SDL_video.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/ImGuiFileBrowser.h"

#include "imgui_shortcut_handler.hpp"

#include "util/bit.hpp"

namespace ImGui {
    IMGUI_API void SetNextWindowRelativePos(const ImGuiViewport *viewport, const ImVec2& pos, ImGuiCond cond = 0, const ImVec2& pivot = ImVec2(0, 0)) {
        SetNextWindowPos({viewport->Pos.x + pos.x,viewport->Pos.y + pos.y});
    }
}

std::pair<ImVec2, ImVec2> get_screen_area(ImVec2 &win_bounds) {
    //auto screen sizing and placement code
    ImVec2 img_bottom_left;
    ImVec2 img_top_right;

    float scaling_factor = min(win_bounds.x/GB_S_W, win_bounds.y/GB_S_H);

    //calculate img size
    img_top_right.x = scaling_factor * GB_S_W;
    img_top_right.y = scaling_factor * GB_S_H;

    img_bottom_left.x = (win_bounds.x - img_top_right.x) / 2;
    img_bottom_left.y = (win_bounds.y - img_top_right.y) / 2;

    img_top_right.x += img_bottom_left.x;
    img_top_right.y += img_bottom_left.y;

    return std::make_pair(img_bottom_left, img_top_right);
}

GUI::GUI(SDL_Window *w, SDL_GLContext g, Configuration *config) :
window(w),
gl_context(g),
rom_file(nullptr),
bios_file(nullptr),
core(nullptr),
config(config) {
    //Build Screen Texture, leave unpopulated right now  so we can do partial updates later
    glGenTextures(1, &screen_texture);                                check_gl_error();
    glBindTexture(GL_TEXTURE_2D, screen_texture);                     check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_gl_error();
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, GB_S_W, GB_S_H);        check_gl_error();
    glBindTexture(GL_TEXTURE_2D, 0);                                  check_gl_error();

    glGenFramebuffers(1, &screen_texture_fbo);                        check_gl_error();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, screen_texture_fbo);       check_gl_error();
    glFramebufferTexture2D(
            GL_READ_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            screen_texture,
            0);                                                       check_gl_error();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
}

GUI::~GUI() {
    //Save Configuration
    config->saveConfigFile("config.cfg"); //TODO: demagic

    delete config;
    if(rom_file) delete rom_file;
    if(core)     delete core;

    //delete custom GL constructs
    glDeleteTextures(1, &screen_texture);
    glDeleteFramebuffers(1, &screen_texture_fbo);

    //shutdown ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    //shutdown SDL
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool GUI::preInitialize() {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();

    return true;
}

GUI *GUI::createGUI(Configuration *config) {
    SDL_Window *window = SDL_CreateWindow("SilverGB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GB_S_W, GB_S_H, SDL_WINDOW_OPENGL);
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    gl3wInit();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

    return new GUI(window, gl_context, config);
}

void GUI::open_file(std::string file) {
    std::cout << "Starting Core" <<std::endl;

    if (config->BIOS.get_bios_loaded()) {
        bios_file = File::openFile(config->BIOS.get_bios_filepath());
        config->BIOS.set_bios_loaded(bios_file != nullptr);
    }

    rom_file = File::openFile(file);
    core = new GB_Core(rom_file, bios_file);
    //core->start_thread(true);
    // state_flags.game_loaded = true;
    //core->set_bp(0x02a5, true);
}

GUI::loop_return_code_t GUI::mainLoop() {
    namespace im = ImGui;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch(event.type) {
        case SDL_QUIT:
            state_flags.loop_finish = true;
            break;
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                state_flags.loop_finish = true;
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if(!im::GetIO().WantCaptureKeyboard
            && !im::BuildShortcut(&event.key)) {
                //do other stuff with the key event
            }
            break;
        }
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    im::NewFrame();

    //Build out the window
    // im::SetNextWindowViewport(im::GetMainViewport()->ID);
    // im::SetNextWindowRelativePos(im::GetMainViewport(), {0.0f, 0.0f});
    // im::SetNextWindowSize(im::GetIO().DisplaySize);
    // im::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    // im::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // im::PushStyleVar(ImGuiStyleVar_WindowPadding, {0,0});
    // im::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    // im::Begin("##Game", nullptr,
    //         ImGuiWindowFlags_NoDecoration      |
    //         ImGuiWindowFlags_NoMove            |
    //         ImGuiWindowFlags_NoScrollWithMouse );

    // im::ShowDemoWindow();

    MainOptionsWindow();

    if(state_flags.open_rom) {
        im::OpenPopup("Open ROM File");
        state_flags.open_rom = false;
    }

    if(state_flags.open_ctxt_menu) {
        state_flags.open_ctxt_menu = false;
    }

    // im::End();
    // im::PopStyleVar(4);

    if(rom_dialog.showFileDialog("Open ROM File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".gb,.bin")) {
        this->open_file(rom_dialog.selected_path);
    }
    if(bios_dialog.showFileDialog("Open BIOS ROM File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".*,.bin")) {
        std::string file = bios_dialog.selected_path;
        File *bios = File::openFile(file);

        if(!bios)
            std::cerr << "bios file could not be opened" << std::endl;
        else {
            if(bios->getCRC() != DMG_BIOS_CRC) {
                std::cerr << std::hex << "bios CRC: " << bios->getCRC() << " != " << DMG_BIOS_CRC << std::dec << std::endl;
                SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_ERROR ,
                        "CRC Mismatch",
                        "CRC does not match known DMG CRC. File will not be loaded.",
                        nullptr);
            } else {
                if(file.size() <= 255) {
                    std::cout << "File loaded" << std::endl;
                    config->BIOS.set_bios_loaded(true);
                    config->BIOS.set_bios_filepath(file.c_str());
                } else {
                    SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_ERROR,
                        "File path is too long",
                        "File path is too long...Maybe the developer should bump up the limit?",
                        nullptr);
                }
            }
        }
        //the bios is opened here to check and save it.
        // the actual loading and reading is done by the core.
        delete bios;
    }

    /**
     * Game Stuff
     */
    if(core) {
        // try {
        //     if(state_flags.run_game) core->tick_frame();
        // }
        // catch(breakpoint_exception &e) {
        //     state_flags.run_game = false;
        // }

        glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
        glTexSubImage2D(
                GL_TEXTURE_2D,            // target
                0,                        // level
                0,                        // xoffset
                0,                        // yoffset
                GB_S_W,                   // width
                GB_S_H,                   // height
                GL_RGB,                   // format
                GL_UNSIGNED_BYTE,         // type
                core->getScreenBuffer());                             check_gl_error();
        glBindTexture(GL_TEXTURE_2D, 0);                              check_gl_error();
    }
    else {
        u8 tsc[160 * 144] = {0};
        u8 *ptr = tsc;
        int x = 0, y = 0, p = 0;
        for(int i = 0; i < (160 * 144); i++) {
            switch(i % 3) {
            case 0:
                *ptr++ = 0xE0;
                break;
            case 1:
                *ptr++ = 0x1C;
                break;
            case 2:
                *ptr++ = 0x02;
                break;
            }
        }
        ptr = tsc;
        glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
        glTexSubImage2D(
                GL_TEXTURE_2D,            // target
                0,                        // level
                0,                        // xoffset
                0,                        // yoffset
                GB_S_W,                   // width
                GB_S_H,                   // height
                GL_RGB,                   // format
                GL_UNSIGNED_BYTE_3_3_2,   // type
                ptr);                                                 check_gl_error();
        glBindTexture(GL_TEXTURE_2D, 0);                              check_gl_error();
    }

    im::ClearShortcut();

    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)im::GetIO().DisplaySize.x, (int)im::GetIO().DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);

    auto area = get_screen_area(im::GetIO().DisplaySize);
    std::cout << area.first.x << " " << area.first.y << " " << area.second.x << " " << area.second.y << std::endl;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, screen_texture_fbo);       check_gl_error();
    glBlitFramebuffer(
            0, 0, GB_S_W, GB_S_H,
            area.first.x, area.first.y, area.second.x, area.second.y,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);                          check_gl_error();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);                        check_gl_error();

    // Rendering
    im::Render();
    ImGui_ImplOpenGL3_RenderDrawData(im::GetDrawData());

    SDL_GL_SwapWindow(window);

    if (im::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        im::UpdatePlatformWindows();
        im::RenderPlatformWindowsDefault();
    }

    if(state_flags.loop_finish) return loop_return_code_t::LOOP_FINISH;
    else                        return loop_return_code_t::LOOP_CONTINUE;
}

void GUI::MainOptionsWindow() {
    namespace im = ImGui;

    static struct {
        bool cpu_register_window = false;
        bool io_register_window = false;
        bool disassemble_window = false;
        bool breakpoint_window = false;
        bool render_window = true;
    } window_flags;

    // im::SetNextWindowViewport(im::GetMainViewport()->ID);
    // im::SetNextWindowRelativePos(im::GetMainViewport(), {0.0f, 0.0f});
    // im::SetNextWindowSize(im::GetIO().DisplaySize);
    // im::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    // im::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // im::PushStyleVar(ImGuiStyleVar_WindowPadding, {0,0});
    // im::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    if(im::Begin("Main Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        if(im::Button("Open ROM")) {
            state_flags.open_rom = true;
        }

        if(im::CollapsingHeader("Emulation Settings")) {
            if(state_flags.game_running) {
                if(im::Button("Pause"))
                    state_flags.game_running = false;
            }
            else {
                if(im::Button("Unpause"))
                    state_flags.game_running = true;
            }
            if(im::Button("tick"))
                core->tick_once();
            if(im::Button("Next Instruction"))
                core->tick_instr();
            if(im::Button("Next Frame"))
                core->tick_frame();
        }

        if(im::CollapsingHeader("Options")) {
            bool bios_loaded = config->BIOS.get_bios_loaded();
            bool bios_enabled = config->BIOS.get_bios_enabled();
            im::MenuItem("BIOS Loaded", nullptr, &bios_loaded, false); // should never need to modify directly so we don't care about writing it back
            im::MenuItem("BIOS Enabled", nullptr, &bios_enabled);
            im::MenuItem("Open BIOS File", NULL, &state_flags.open_bios);
            // im::MenuItem("Debug Mode", NULL, &state_flags.set_debug_mode);
            im::EndMenu();

            config->BIOS.set_bios_enabled(bios_enabled);
        }

        if(state_flags.debug_mode) {
            if(im::BeginMenu("Debug")) {
                im::MenuItem("CPU Register Window", nullptr, &window_flags.cpu_register_window);
                im::MenuItem("IO Register Window", nullptr, &window_flags.io_register_window);
                im::MenuItem("Hide Disassembly Window", nullptr, &window_flags.disassemble_window);
                im::MenuItem("Breakpoint Window",nullptr, &window_flags.breakpoint_window);

                im::EndMenu();
            }
        }
        ImGui::End();
    }
    // im::PopStyleVar(2);

    // process shortcuts
    state_flags.open_rom = state_flags.open_rom || im::ShortcutPressed({ im::CTRL_MOD },'f');

    //TODO: better check for game running
    // if(core) {
    //     if(im::ShortcutPressed({im::CTRL_MOD}, 'd')) core->getByteFromIO(0);

    //     try {
    //         if(im::ShortcutPressed({im::CTRL_MOD}, 't')) core->tick_instr();
    //         if(im::ShortcutPressed({im::CTRL_MOD, im::ALT_MOD}, 'f')) core->tick_frame();
    //         if(im::ShortcutPressed({im::CTRL_MOD, im::SHIFT_MOD}, 't')) {
    //             for(int i = 0; i < 0x100; i++)
    //                 core->tick_instr();
    //         }
    //     } catch(breakpoint_exception &e) {
    //         SDL_ShowSimpleMessageBox(
    //             SDL_MESSAGEBOX_ERROR ,
    //             "Breakpoint",
    //             "You've Hit a breakpoint!",
    //             nullptr);
    //     }
    // }

    if(window_flags.cpu_register_window) buildCPURegisterWindow();
    if(window_flags.io_register_window)  buildIORegisterWindow();
    if(window_flags.disassemble_window)  buildDisassemblyWindow();
    if(window_flags.breakpoint_window)   buildBreakpointWindow();
}

void GUI::buildCPURegisterWindow() {
    // using namespace ImGui;

    // static struct {
    //     bool AF;
    //     bool F;
    //     bool BC;
    //     bool DE;
    //     bool HL;
    // } reg_flags;

    // CPU::registers_t regs = { 0 };
    // if(core) regs = core->getRegistersFromCPU();
    // else     regs = { 0 };

    // //The Indentation on this window is very sensitive...
    // SetNextWindowSize({120,0});
    // Begin("Registers", nullptr, ImGuiWindowFlags_NoResize);
    // Unindent(10.0);
    // Columns(2, nullptr, true);
    // SetColumnWidth(-1, 60);
    // if(TreeNode("AF")) {
    //     reg_flags.AF = true;
    //     Indent(5.0);
    //     Text("A");
    //     Unindent(20.0);
    //     if(TreeNode("F")) {
    //         reg_flags.F = true;
    //         Indent(5.0);
    //         Text("Z");
    //         Text("N");
    //         Text("H");
    //         Text("C");
    //         Unindent(5.0);
    //         TreePop();
    //     } else {
    //         reg_flags.F = false;
    //     }
    //     Indent(15.0);
    //     TreePop();
    // } else {
    //     reg_flags.F = false;
    //     reg_flags.AF = false;
    // }

    // if(TreeNode("BC")) {
    //     reg_flags.BC = true;
    //     Indent(5.0);
    //     Text("B");
    //     Text("C");
    //     Unindent(5.0);
    //     TreePop();
    // } else {
    //     reg_flags.BC = false;
    // }

    // if(TreeNode("DE")) {
    //     reg_flags.DE = true;
    //     Indent(5.0);
    //     Text("D");
    //     Text("E");
    //     Unindent(5.0);
    //     TreePop();
    // } else {
    //     reg_flags.DE = false;
    // }

    // if(TreeNode("HL")) {
    //     reg_flags.HL = true;
    //     Indent(5.0);
    //     Text("H");
    //     Text("L");
    //     Unindent(5.0);
    //     TreePop();
    // } else {
    //     reg_flags.HL = false;
    // }
    // Indent(20.0);
    // Text("PC");
    // Text("SP");
    // Unindent(20.0);
    // if(reg_flags.F) {
    //     SetColumnWidth(-1, 60);
    // } else {
    //     SetColumnWidth(-1, 50);
    // }
    // NextColumn();
    // SetColumnWidth(-1, 50);

    // //AF
    // Text("0x%s", itoh(regs.AF, 4).c_str());
    // if(reg_flags.AF) {
    //     Text("0x%s", itoh(regs.AF >> 8, 2).c_str());  //A
    //     Text("0x%s", itoh(regs.AF & 0xFF, 2).c_str()); //F
    //     if(reg_flags.F) {
    //         Text("%u", Bit.test(regs.AF, 7)); //Z
    //         Text("%u", Bit.test(regs.AF, 6)); //N
    //         Text("%u", Bit.test(regs.AF, 5)); //H
    //         Text("%u", Bit.test(regs.AF, 4)); //C
    //     }
    // }

    // //BC
    // Text("0x%s", itoh(regs.BC, 4).c_str());
    // if(reg_flags.BC) {
    //     Text("0x%s", itoh(regs.BC >> 8, 2).c_str());  //B
    //     Text("0x%s", itoh(regs.BC & 0xFF, 2).c_str()); //C
    // }

    // //DE
    // Text("0x%s", itoh(regs.DE, 4).c_str());
    // if(reg_flags.DE) {
    //     Text("0x%s", itoh(regs.DE >> 8, 2).c_str());  //D
    //     Text("0x%s", itoh(regs.DE & 0xFF, 2).c_str()); //E
    // }

    // //HL
    // Text("0x%s", itoh(regs.HL, 4).c_str());
    // if(reg_flags.HL) {
    //     Text("0x%s", itoh(regs.HL >> 8, 2).c_str());  //H
    //     Text("0x%s", itoh(regs.HL & 0xFF, 2).c_str()); //L
    // }

    // //PC
    // Text("0x%s", itoh(regs.PC, 4).c_str());

    // //SP
    // Text("0x%s", itoh(regs.SP, 4).c_str());
    // End();
}

void GUI::buildIORegisterWindow() {
    // using namespace ImGui;

    // IO_Bus::io_registers_t regs = { 0 };
    // if(core) regs = core->getregistersfromIO();
    // else     regs = { 0 };

    // //The Indentation on this window is very sensitive...
    // SetNextWindowSize({120,0});
    // Begin("IO Registers", nullptr, ImGuiWindowFlags_NoResize);
    // Columns(2, nullptr, true);
    // SetColumnWidth(-1, 60);

    // Text("P1");
    // Text("SB");
    // Text("SC");
    // Text("DIV");
    // Text("TIMA");
    // Text("TMA");
    // Text("TAC");
    // Text("IF");
    // Text("LCDC");
    // Text("STAT");
    // Text("SCY");
    // Text("SCX");
    // Text("LY");
    // Text("LYC");
    // Text("DMA");
    // Text("BGP");
    // Text("OBP0");
    // Text("OBP1");
    // Text("WY");
    // Text("WX");
    // Text("VBK");
    // Text("SVBK");
    // Text("IE");
    // NextColumn();
    // SetColumnWidth(-1, 50);

    // Text("0x%s", itoh(regs.P1, 2).c_str());
    // Text("0x%s", itoh(regs.SB, 2).c_str());
    // Text("0x%s", itoh(regs.SC, 2).c_str());
    // Text("0x%s", itoh(regs.DIV, 2).c_str());
    // Text("0x%s", itoh(regs.TIMA, 2).c_str());
    // Text("0x%s", itoh(regs.TMA, 2).c_str());
    // Text("0x%s", itoh(regs.TAC, 2).c_str());
    // Text("0x%s", itoh(regs.IF, 2).c_str());
    // Text("0x%s", itoh(regs.LCDC, 2).c_str());
    // Text("0x%s", itoh(regs.STAT, 2).c_str());
    // Text("0x%s", itoh(regs.SCY, 2).c_str());
    // Text("0x%s", itoh(regs.SCX, 2).c_str());
    // Text("0x%s", itoh(regs.LY, 2).c_str());
    // Text("0x%s", itoh(regs.LYC, 2).c_str());
    // Text("0x%s", itoh(regs.DMA, 2).c_str());
    // Text("0x%s", itoh(regs.BGP, 2).c_str());
    // Text("0x%s", itoh(regs.OBP0, 2).c_str());
    // Text("0x%s", itoh(regs.OBP1, 2).c_str());
    // Text("0x%s", itoh(regs.WY, 2).c_str());
    // Text("0x%s", itoh(regs.WX, 2).c_str());
    // Text("0x%s", itoh(regs.VBK, 2).c_str());
    // Text("0x%s", itoh(regs.SVBK, 2).c_str());
    // Text("0x%s", itoh(regs.IE, 2).c_str());
    // End();
}

void GUI::buildDisassemblyWindow() {
    // using namespace ImGui;

    // SetNextWindowSize({120,0});
    // Begin("Disassembly", nullptr, ImGuiWindowFlags_NoResize);
    // End();
}

// void GUI::buildMemoryViewWindow() {
//     //NOTE: broken. I'm just tired of looking at it ATM.
//     using namespace ImGui;

//     //disable the window grip in the bottom left corner
//     //TODO: find a cleaner way to do this, I've contacted the dev for suggestions
//     PushStyleColor(ImGuiCol_ResizeGrip,        GetColorU32(ImGuiCol_WindowBg));
//     PushStyleColor(ImGuiCol_ResizeGripActive,  GetColorU32(ImGuiCol_WindowBg));
//     PushStyleColor(ImGuiCol_ResizeGripHovered, GetColorU32(ImGuiCol_WindowBg));

//     SetNextWindowSizeConstraints({410, 202}, {410, FLT_MAX});
//     Begin("Memory View", nullptr, ImGuiWindowFlags_NoScrollWithMouse);
//     static char start_buf[5] = { 0 };
//     static char end_buf[5] = { 0 };
//     int start_int, end_int;

//     ImGuiInputTextCallback text_func =
//             [](ImGuiInputTextCallbackData *data) -> int {
//                 switch(data->EventChar) {
//                 case '0' ... '9':
//                     break;
//                 case 'a' ... 'f':
//                     data->EventChar -= ('a' - 'A');
//                     break;
//                 case 'A' ... 'F':
//                     break;
//                 default:
//                     return 1;
//                     break;
//                 }
//                 return 0;
//             };

//     if(BeginChild("##")) {
//         PushItemWidth(50);
//         InputText("Start",
//                 start_buf,
//                 5,
//                 ImGuiInputTextFlags_CallbackCharFilter,
//                 text_func,
//                 nullptr);
//         InputText("End",
//                 end_buf,
//                 5,
//                 ImGuiInputTextFlags_CallbackCharFilter,
//                 text_func,
//                 nullptr);
//         PopItemWidth();

//         start_int = htoi(start_buf);
//         end_int   = htoi(end_buf);
//     }
//     EndChild();

//     if(BeginChild("##")) {
//         //TODO:
//         /**the listbox works by creating it's a customized child window.
//          * since the listbox doesn't take an "additional flags" parameter I can't control
//          * how the scrollbar shows up.
//          *
//          * Ideally I force it to be omni-present and design the
//          * sizing of the window around it. for now this isn't an option so heres a hack to
//          * remove it entirely.
//          *
//          * The section is still scrollable with the mouse-wheel which is good enough for now
//          **/
//         PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
//         if(ListBoxHeader("##", {-1,-1})) {
//             Columns(2);
//             SetColumnWidth(-1, 57);
//                 TextUnformatted("");
//             for(int i = 0; i < ((end_int+0x10)&0xFFF0) - (start_int&0xFFF0); i+= 0x10) {
//                 Text("0x%04X", i+(start_int & 0xFFF0));
//             }
//             NextColumn();
//             TextUnformatted("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
//             Separator();

//             //TODO: optimize this code with ImGUI list clipper
//             if(core && start_int < end_int) {
//                 int start_floor      = start_int     & 0xFFF0,
//                     start_floor_diff = start_int     & 0xF,
//                     end_floor        = (end_int)     & 0xFFF0,
//                     end_floor_diff   = end_int       & 0xF,
//                     end_ceiling      = (end_int+0xF) & 0xFFF0;

//                 if(start_floor != start_int) {
//                     char  c[48] = { 0 };
//                     char *c_index = c;
//                     for(int i = 0; i < start_floor_diff; i++) {
//                         *c_index++ = *c_index++ = *c_index++ = ' ';
//                     }

//                     if(start_floor + 0x10 < end_int) {
//                         for(int i = start_floor_diff; i < 0x10; i++) {
//                             memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2, true).c_str(), 3);
//                             *++c_index = ' ';
//                             c_index++;
//                         }
//                     } else {
//                         for(int i = start_floor_diff; i < end_int; i++) {
//                             memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2, true).c_str(), 3);
//                             *++c_index = ' ';
//                             c_index++;
//                         }
//                     }
//                     TextUnformatted(c);
//                 }

//                 if(start_floor + 0x10 < end_int) {
//                     for(int i = start_floor + 0x10; i < end_ceiling-0x10; i+=0x10) {
//                         char  c[48] = { 0 };
//                         char *c_index = c;
//                         for(int j = i; j < i+0x10; j++) {
//                             memcpy(c_index++, itoh(core->getByteFromIO(i+j), 2, true).c_str(), 3);
//                             *++c_index = ' ';
//                             c_index++;
//                         }
//                         TextUnformatted(c);
//                     }

//                     if(end_floor != end_ceiling) {
//                         char  c[48] = { 0 };
//                         char *c_index = c;
//                         for(int i = end_floor; i < end_int; i++) {
//                             memcpy(c_index++, itoh(core->getByteFromIO(i), 2, true).c_str(), 3);
//                             *++c_index = ' ';
//                             c_index++;
//                         }
//                         TextUnformatted(c);
//                     }
//                 }
//             }
//             ListBoxFooter();
//         }
//         PopStyleVar();
//     }
//     EndChild();
//     End();
//     PopStyleColor(3);
// }

    //TODO: fix
void GUI::buildBreakpointWindow() {
    // using namespace ImGui;

    // ImGuiInputTextCallback text_func =
    //     [](ImGuiInputTextCallbackData *data) -> int {
    //         switch(data->EventChar) {
    //         case '0' ... '9':
    //             break;
    //         case 'a' ... 'f':
    //             data->EventChar -= ('a' - 'A');
    //             break;
    //         case 'A' ... 'F':
    //             break;
    //         default:
    //             return 1;
    //             break;
    //         }
    //         return 0;
    //     };

    // static char buf[5] = "";

    // if(Begin("Breakpoint", nullptr, ImGuiWindowFlags_NoResize)) {
    //     Columns(2, nullptr, true);
    //         SetColumnWidth(-1, 90);
    //         Dummy({0,0.25});
    //         Text("Breakpoint:");
    //         Dummy({0,0.25});
    //         Text("Enabled:");
    //     NextColumn();
    //         PushItemWidth(50);
    //         InputText("##", buf, 5, ImGuiInputTextFlags_CallbackCharFilter, text_func);
    //         //SetItemDefaultFocus();
    //         PopItemWidth();

    //         bool breakpoint_enabled = false;
    //         if(core) {
    //             breakpoint_enabled = core->get_bp_active();
    //         }

    //         if(Checkbox("##", &breakpoint_enabled)) {
    //             core->set_bp_active(breakpoint_enabled);
    //         }
    // }
    // End();
}