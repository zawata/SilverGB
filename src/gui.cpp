//#include <assert.h>

#include "gui.hpp"

#include "util/bit.hpp"

/**
 *  A small shortcut handler built in the image of ImGui.
 *
 * ImGui works on a technical basis by combining gui logic and construction.
 * Any call to a GUI component will display that GUI component as well as checking
 * if it's particular logic has been met. It does this with internal static
 * variables to handle the rendering process and assertions to handle logical
 * errors(making them rather easy to find)
 *
 * We can build a small shortcut handler in the same image.

 * At the beginning of every frame we can process the key events and build a key combination
 * then every component we want to have a shortcut just creates it's GUI Component and
 * checks it's shortcut in an `if` statement.
 * not very complex but effective.
 *
 * This does pose issues with using keyboard events and shortcuts simultaneously as we
 * might accidentally attempt to use keyboard events as shortcuts unintentionally.
 *
 * We can overcome this by checking if a keyevent has a modifier on it and denying all
 * shortcut requests without a modifier key.
 *
 **/
bool GUI::build_shortcut(SDL_KeyboardEvent *key) {
    if(     key->keysym.sym >= 32 &&  //if ascii
            key->keysym.sym <= 122 &&
            key->keysym.mod &&        //if it has a modifier
            key->state == SDL_PRESSED) { //if it's pressed(not released)
        if(     key->keysym.mod & KMOD_CTRL ||
                key->keysym.mod & KMOD_ALT ||
                key->keysym.mod & KMOD_SHIFT) {
            this->current_shortcut = {
                .valid     = true,
                .ctrl_mod  = key->keysym.mod & KMOD_CTRL,
                .alt_mod   = key->keysym.mod & KMOD_ALT,
                .shift_mod = key->keysym.mod & KMOD_SHIFT,
                .key       = (char)key->keysym.sym,
            };
            return true;
        }
    }
    return false;
}

void GUI::clear_shortcut() {
    this->current_shortcut = {
        .valid = false,
    };
}

/**
 * Creating a struct for the modifier keys allows us to use initializer lists
 * to pass in modifiers.
 *
 * so instead of doing seomthign like:
 *
 *    bool shortcut_pressed(bool ctrl_key, bool alt_key, bool shift_key, char key);
 *
 *    shortcut_pressed(true, false, true, 'c'); // CTRL+SHIFT+C
 *
 * we can do:
 *
 *    bool shortcut_pressed(mod_arg mods, char key);
 *
 *    shortcut_pressed({ CTRL, SHIFT }, 'c'); // CTRL+SHIFT+C
 *
 * Which I think looks cleaner.
 *
 * The processing for doing this is a little wierder though.
 *
 * As it is designed:
 *      3 conditions have to be created. the conditions being each of
 *      the modifier keys
 *
 *      if the current shortcut doesn't need a particular modifier, then the
 *      condition is marked as fulfilled.
 *
 *      then we "loop" through the variable sin the modifier struct
 *      and mark each condition they match as fulfilled
 *
 *      if a condition is already fulfilled and we match it,
 *      then the current shortcut wasn't using this modifier and
 *      we don't match the shortcut
 *
 *      after each field is processed, we check that all conditions are
 *      marked as fulfilled to make sure the shortcut doesn't require additional
 *      modifiers that we don't have.
 *
 *      lastly the base key is checked to see if it matches.
 *
 */
bool GUI::shortcut_pressed(const mod_arg mods, char key) {
    assert(mods.mod0 != 0); //all shortcuts require a modifier

    //check if the shortcut is valid before processing it
    if(!this->current_shortcut.valid) return false;

    //setup condition flags.
    bool mod_ctrl_handled  = !current_shortcut.ctrl_mod,
         mod_alt_handled   = !current_shortcut.alt_mod,
         mod_shift_handled = !current_shortcut.shift_mod;

    for(int i = 0; i < 3; i++) {
        u8 mod;
        //iterate over the struct fields
        switch(i) {
            case 0: mod = mods.mod0; break;
            case 1: mod = mods.mod1; break;
            case 2: mod = mods.mod2; break;
        }

        switch(mod) {
        case 0:
            break;
        case mod_key::CTRL:
            if(mod_ctrl_handled) return false;
            else mod_ctrl_handled = true;
            break;
        case mod_key::ALT:
            if(mod_alt_handled) return false;
            else mod_alt_handled = true;
            break;
        case mod_key::SHIFT:
            if(mod_shift_handled) return false;
            else mod_shift_handled = true;
            break;
        default:
            return false;
        }
    }

    return key == current_shortcut.key &&
        mod_ctrl_handled &&
        mod_alt_handled  &&
        mod_shift_handled;
}

GUI::GUI(SDL_Window *w, SDL_GLContext g, ImGuiIO &io) :
window(w),
gl_context(g),
io(io),
rom_file(nullptr),
core(nullptr), //if this isn't here, the core will break wildly.
screen_buffer((u8 *)malloc(GB_S_P_SZ)) {

    io.IniFilename = nullptr; //disable IMGUI ini file. //TODO: re-evaluate later

    config = Configuration::loadConfigFile("config.cfg");
    if(!config) {
        std::cout << "creating new Config" << std::endl;
        //the config doesn't exist so lets just start a new one
        config = new Configuration();
    }

    glGenTextures(1, &screen_texture);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GUI::~GUI() {
    //Save Configuration
    config->saveConfigFile("config.cfg");

    delete config;
    if(rom_file) delete rom_file;
    if(core)     delete core;

    //delete custom textures
    glDeleteTextures(1, &screen_texture);

    //shutdown ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    //shutdown SDL
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    free(screen_buffer);
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

GUI *GUI::createGUI() {
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window *window = SDL_CreateWindow("SilverBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GB_S_W, GB_S_H + MENUBAR_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    gl3wInit();

    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    return new GUI(window, gl_context, ImGui::GetIO());
}

GUI::loop_return_code_t GUI::mainLoop() {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch(event.type) {
        case SDL_QUIT:
            state_flags.done = true;
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                state_flags.done = true;
            }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if(!io.WantCaptureKeyboard) {
                if(!build_shortcut(&event.key)) {
                    //do other stuff with the key event
                }
            }
        }
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    //build the UI
    buildUI();

    if(state_flags.opening_file || shortcut_pressed({ CTRL },'f')) {
        char *file = nullptr;
        if(NFD_OpenDialog("gb,bin", nullptr, &file) == NFD_OKAY) {
            std::cout << "Starting Core" << std::endl;
            rom_file = File_Interface::openFile(std::string(file));
            core = new GB_Core(rom_file, config);
            core->start(true);
            state_flags.game_loaded = true;
            free(file);
        }
        state_flags.opening_file = false;
    }

    if(core) {
        if(shortcut_pressed({CTRL}, 't')) {
            //core->tick();
        }
        if(shortcut_pressed({CTRL, SHIFT}, 't')) {
            //for(int i = 0; i < 0x80; i++) core->tick();
        }
    }

    if(state_flags.opening_bios) {
        char *file = nullptr;
        if(NFD_OpenDialog("bin", nullptr, &file) == NFD_OKAY) {
            File_Interface *bios = File_Interface::openFile(file);

            if(!bios) std::cerr << "bios file could not be opened" << std::endl;
            else {
                if(bios->getCRC() != DMG_BIOS_CRC) {
                    std::cerr << std::hex << "bios CRC: " << bios->getCRC() << " != " << DMG_BIOS_CRC << std::dec << std::endl;
                    SDL_ShowSimpleMessageBox(
                            SDL_MESSAGEBOX_ERROR ,
                            "CRC Mismatch",
                            "CRC does not match known DMG CRC. File will not be loaded.",
                            nullptr);
                } else {
                    int file_len = std::string(file).size();
                    if(file_len <= 255) {
                        std::cout << "File loaded" << std::endl;
                        config->config_data.bin_enabled = true;
                        memcpy(config->config_data.bin_file, file, file_len);
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
            free(file);
        }
        state_flags.opening_bios = false;
    }

    if(state_flags.set_debug_mode && !state_flags.debug_mode) {
        //set debug mode
        SDL_SetWindowSize(this->window, 640,480);
        SDL_SetWindowResizable(this->window, SDL_TRUE);
        state_flags.debug_mode = true;
    } else if(!state_flags.set_debug_mode && state_flags.debug_mode) {
        //unset debug mode
        SDL_SetWindowSize(this->window, GB_S_W, GB_S_H + MENUBAR_HEIGHT);
        SDL_SetWindowResizable(this->window, SDL_FALSE);
        state_flags.debug_mode = false;
    }

    clear_shortcut();

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);

    if(state_flags.done) return loop_return_code_t::LOOP_FINISH;
    else                 return loop_return_code_t::LOOP_CONTINUE;
}

void GUI::buildUI() {
    using namespace ImGui;

    static struct {
        bool render_window = true;
        bool register_window = false;
        bool disassemble_window = false;
        bool VRAMview_window = false;
    } window_flags;

    if(BeginMainMenuBar()) {
        if(BeginMenu("File")) {
            MenuItem("Open File", NULL, &state_flags.opening_file);
            MenuItem("Exit", NULL, &state_flags.done);
            EndMenu();
        }

        if(state_flags.game_loaded) {
            if(BeginMenu("Emulation")) {
                if(core->paused()) {
                    if(MenuItem("Unpause")) core->resume();
                    //if(MenuItem("tick")) core->tick();
                }
                else {
                    if(MenuItem("Pause")) core->pause();
                }
                EndMenu();
            }
        }

        if(BeginMenu("Options")) {
            MenuItem("BIOS File", nullptr, &config->config_data.bin_enabled, false);
            MenuItem("Open BIOS File", NULL, &state_flags.opening_bios);
            MenuItem("Debug Mode", NULL, &state_flags.set_debug_mode);
            EndMenu();
        }

        if(state_flags.debug_mode) {
            if(BeginMenu("Debug")) {
                if(window_flags.register_window) {
                    if(MenuItem("Hide Register Window")) window_flags.register_window = false;
                } else {
                    if(MenuItem("Show Register Window")) window_flags.register_window = true;
                }

                if(window_flags.disassemble_window) {
                    if(MenuItem("Hide Disassembly Window")) window_flags.disassemble_window = false;
                } else {
                    if(MenuItem("Show Disassembly Window")) window_flags.disassemble_window = true;
                }

                if(window_flags.VRAMview_window) {
                    if(MenuItem("Hide Memory View Window")) window_flags.VRAMview_window = false;
                } else {
                    if(MenuItem("Show Memory View Window")) window_flags.VRAMview_window = true;
                }

                EndMenu();
            }
        }
        EndMainMenuBar();
    }

    if(window_flags.register_window) buildRegisterUI();
    if(window_flags.disassemble_window) buildDisassemblyUI();
    if(window_flags.render_window) buildRenderUI();
    if(window_flags.VRAMview_window) buildMemoryViewUI();
}

void GUI::buildRenderUI() {
    using namespace ImGui;

    if(!state_flags.debug_mode) {
        SetNextWindowSize({GB_S_W, GB_S_H});
        SetNextWindowPos({0, MENUBAR_HEIGHT});

        PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        PushStyleVar(ImGuiStyleVar_WindowPadding, {0,0});
        PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        Begin("Game", nullptr,
                ImGuiWindowFlags_NoTitleBar        |
                ImGuiWindowFlags_NoResize          |
                ImGuiWindowFlags_NoMove            |
                ImGuiWindowFlags_NoScrollbar       |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoCollapse);
    } else {
        SetNextWindowSize({0,0});
        PushStyleVar(ImGuiStyleVar_WindowPadding, {8,0});
        Begin("Game", nullptr,
                ImGuiWindowFlags_NoCollapse  |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse);
    }

    //TODO: update screen_buffer with pixel data from VPU
    memset(screen_buffer, 00, GB_S_P_SZ);
    for(int i = 0; i < GB_S_P_SZ; i+=3) {
        //for now just draw red.
        screen_buffer[i] = 0xFF;
        screen_buffer[i+1] = 0x00;
        screen_buffer[i+2] = 0x00;
    }
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexImage2D(
            GL_TEXTURE_2D,          //target
            0,                      //level
            GL_RGB8,                //base_format
            GB_S_W,                 //width
            GB_S_H,                 //height
            0,                      //border
            GL_RGB,                 //color_format
            GL_UNSIGNED_BYTE,       //data_format
            (void *)screen_buffer); //pixel_buffer
    glBindTexture(GL_TEXTURE_2D, 0);
    ImGui::Image((void *)(intptr_t)screen_texture, {GB_S_W, GB_S_H});
    End();

    //auto screen sizing and placement code
    // if(window.w > window.h) {
    //     //more wide than tall
    //     pixel_size = (window.h/144);

    //     image.w = pixel_size * 160;
    //     image.h = pixel_size * 144;

    //     image.x = (window.w/2) - (image.w/2);
    //     image.y = 0;

    // } else if(window.w < window.h) {
    //     //more wide than tall
    //     pixel_size = (window.w/160);

    //     image.w = pixel_size * 160;
    //     image.h = pixel_size * 144;

    //     image.x = 0;
    //     image.y = (window.h/2) - (image.h/2);
    // } else {
    //     //perfect square
    //     pixel_size = (window.h/144);

    //     image.w = pixel_size * 160;
    //     image.h = pixel_size * 144;

    //     image.x = 0;
    //     image.y = 0;
    // }

    if(!state_flags.debug_mode) {
        PopStyleVar(4);
    } else {
        PopStyleVar(1);
    }
}

void GUI::buildRegisterUI() {
    using namespace ImGui;

    static struct {
        bool AF;
        bool F;
        bool BC;
        bool DE;
        bool HL;
    } reg_flags;

    CPU::registers_t regs = { 0 };
    if(core) regs = core->getRegistersFromCPU();
    else     regs = { 0 };

    //The Indentation on this window is very sensitive...
    SetNextWindowSize({120,0});
    Begin("Registers", nullptr, ImGuiWindowFlags_NoResize);
    Unindent(10.0);
    Columns(2, nullptr, true);
    SetColumnWidth(-1, 60);
    if(TreeNode("AF")) {
        reg_flags.AF = true;
        Indent(5.0);
        Text("A");
        Unindent(20.0);
        if(TreeNode("F")) {
            reg_flags.F = true;
            Indent(5.0);
            Text("C");
            Text("H");
            Text("N");
            Text("Z");
            Unindent(5.0);
            TreePop();
        } else {
            reg_flags.F = false;
        }
        Indent(15.0);
        TreePop();
    } else {
        reg_flags.F = false;
        reg_flags.AF = false;
    }

    if(TreeNode("BC")) {
        reg_flags.BC = true;
        Indent(5.0);
        Text("B");
        Text("C");
        Unindent(5.0);
        TreePop();
    } else {
        reg_flags.BC = false;
    }

    if(TreeNode("DE")) {
        reg_flags.DE = true;
        Indent(5.0);
        Text("D");
        Text("E");
        Unindent(5.0);
        TreePop();
    } else {
        reg_flags.DE = false;
    }

    if(TreeNode("HL")) {
        reg_flags.HL = true;
        Indent(5.0);
        Text("H");
        Text("L");
        Unindent(5.0);
        TreePop();
    } else {
        reg_flags.HL = false;
    }
    Indent(20.0);
    Text("PC");
    Text("SP");
    Unindent(20.0);
    if(reg_flags.F) {
        SetColumnWidth(-1, 60);
    } else {
        SetColumnWidth(-1, 50);
    }
    NextColumn();
    SetColumnWidth(-1, 50);

    //AF
    Text("0x%s", itoh(regs.AF, 4).c_str());
    if(reg_flags.AF) {
        Text("0x%s", itoh(regs.AF >> 8, 2).c_str());  //A
        Text("0x%s", itoh(regs.AF & 0xFF, 2).c_str()); //F
        if(reg_flags.F) {
            Text("%u", Bit.test(regs.AF, 3)); //C
            Text("%u", Bit.test(regs.AF, 2)); //H
            Text("%u", Bit.test(regs.AF, 1)); //N
            Text("%u", Bit.test(regs.AF, 0)); //Z
        }
    }

    //BC
    Text("0x%s", itoh(regs.BC, 4).c_str());
    if(reg_flags.BC) {
        Text("0x%s", itoh(regs.BC >> 8, 2).c_str());  //B
        Text("0x%s", itoh(regs.BC & 0xFF, 2).c_str()); //C
    }

    //DE
    Text("0x%s", itoh(regs.DE, 4).c_str());
    if(reg_flags.DE) {
        Text("0x%s", itoh(regs.DE >> 8, 2).c_str());  //D
        Text("0x%s", itoh(regs.DE & 0xFF, 2).c_str()); //E
    }

    //HL
    Text("0x%s", itoh(regs.HL, 4).c_str());
    if(reg_flags.HL) {
        Text("0x%s", itoh(regs.HL >> 8, 2).c_str());  //H
        Text("0x%s", itoh(regs.HL & 0xFF, 2).c_str()); //L
    }

    //PC
    Text("0x%s", itoh(regs.PC, 4).c_str());

    //SP
    Text("0x%s", itoh(regs.SP, 4).c_str());
    End();
}

void GUI::buildDisassemblyUI() {
    using namespace ImGui;

    SetNextWindowSize({120,0});
    Begin("Disassembly", nullptr, ImGuiWindowFlags_NoResize);
    End();
}

void GUI::buildMemoryViewUI() {
    using namespace ImGui;

    //disable the window grip in the bottom left corner
    PushStyleColor(ImGuiCol_ResizeGrip,        0);
    PushStyleColor(ImGuiCol_ResizeGripActive,  0);
    PushStyleColor(ImGuiCol_ResizeGripHovered, 0);

    SetNextWindowSizeConstraints({410, 202}, {410, FLT_MAX});
    Begin("Memory View", nullptr, ImGuiWindowFlags_NoScrollWithMouse);
    static char start_buf[5] = { 0 };
    static char end_buf[5] = { 0 };
    int start_int, end_int;

    ImGuiInputTextCallback text_func =
            [](ImGuiInputTextCallbackData *data) -> int {
                switch(data->EventChar) {
                case '0' ... '9':
                    break;
                case 'a' ... 'f':
                    data->EventChar -= ('a' - 'A');
                    break;
                case 'A' ... 'F':
                    break;
                default:
                    return 1;
                    break;
                }
                return 0;
            };

    if(BeginChild("##")) {
        PushItemWidth(50);
        InputText("Start",
                start_buf,
                5,
                ImGuiInputTextFlags_CallbackCharFilter,
                text_func,
                nullptr);
        InputText("End",
                end_buf,
                5,
                ImGuiInputTextFlags_CallbackCharFilter,
                text_func,
                nullptr);
        PopItemWidth();

        start_int = htoi(start_buf);
        end_int   = htoi(end_buf);
    }
    EndChild();

    if(BeginChild("##")) {
        //TODO:
        /**the listbox works by creating a customized child window.
         * since the listbox doesn't take an "additional flags" parameter I can't control
         * how the scrollbar shows up.
         *
         * Ideally I force it to be omni-present and design the
         * sizing of the window around it. for now this isn't an option so heres a hack to
         * remove it entirely.
         *
         * The section is still scrollable with the mouse-wheel which is good enough for now
         **/
        PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
        if(ListBoxHeader("##", {-1,-1})) {
            Columns(2);
            SetColumnWidth(-1, 57);
                TextUnformatted("");
            for(int i = 0; i < ((end_int+0x10)&0xFFF0) - (start_int&0xFFF0); i+= 0x10) {
                Text("0x%04X", i+(start_int & 0xFFF0));
            }
            NextColumn();
            TextUnformatted("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
            Separator();

            //TODO: optimize this code with ImGUI list clipper
            if(core && start_int < end_int) {
                int start_floor      = start_int     & 0xFFF0,
                    start_floor_diff = start_int     & 0xF,
                    end_floor        = (end_int)     & 0xFFF0,
                    //end_floor_diff   = end_int       & 0xF,
                    end_ceiling      = (end_int+0xF) & 0xFFF0;

                if(start_floor != start_int) {
                    char  c[48] = { 0 };
                    char *c_index = c;
                    for(int i = 0; i < start_floor_diff; i++) {
                        *c_index++ = ' ';
                        *c_index++ = ' ';
                        *c_index++ = ' ';
                    }

                    if(start_floor + 0x10 < end_int) {
                        for(int i = start_floor_diff; i < 0x10; i++) {
                            memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2, true).c_str(), 3);
                            *++c_index = ' ';
                            c_index++;
                        }
                    } else {
                        for(int i = start_floor_diff; i < end_int; i++) {
                            memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2, true).c_str(), 3);
                            *++c_index = ' ';
                            c_index++;
                        }
                    }
                    TextUnformatted(c);
                }

                if(start_floor + 0x10 < end_int) {
                    for(int i = start_floor + 0x10; i < end_ceiling-0x10; i+=0x10) {
                        char  c[48] = { 0 };
                        char *c_index = c;
                        for(int j = i; j < i+0x10; j++) {
                            memcpy(c_index++, itoh(core->getByteFromIO(i+j), 2, true).c_str(), 3);
                            *++c_index = ' ';
                            c_index++;
                        }
                        TextUnformatted(c);
                    }

                    if(end_floor != end_ceiling) {
                        char  c[48] = { 0 };
                        char *c_index = c;
                        for(int i = end_floor; i < end_int; i++) {
                            memcpy(c_index++, itoh(core->getByteFromIO(i), 2, true).c_str(), 3);
                            *++c_index = ' ';
                            c_index++;
                        }
                        TextUnformatted(c);
                    }
                }
            }
            ListBoxFooter();
        }
        PopStyleVar();
    }
    EndChild();
    //hide diagonal mouse cursors
    if (GetMouseCursor() == ImGuiMouseCursor_ResizeNWSE || GetMouseCursor() == ImGuiMouseCursor_ResizeNESW)
        SetMouseCursor(ImGuiMouseCursor_Arrow);
    End();
    PopStyleColor(3);
}