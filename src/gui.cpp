#include "gui.hpp"

GUI::GUI(SDL_Window *w, SDL_GLContext g, ImGuiIO &io) :
window(w),
gl_context(g),
io(io),
screen_buffer((u8 *)malloc(GB_S_P_SZ)) {

    glGenTextures(1, &screen_texture);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GUI::~GUI() {
    //delete custom textures
    glDeleteTextures(1, &screen_texture);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    return new GUI(window, gl_context, io);
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
        if (event.type == SDL_QUIT)
            state_flags.done = true;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            state_flags.done = true;
    }

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    buildUI();
    //ImGui::ShowMetricsWindow();

    if(state_flags.opening_file) {
        char *file = nullptr;
        if(NFD_OpenDialog("gb,bin", nullptr, &file) == NFD_OKAY) {
            printf("%s\n", file); //TODO: heres the file. do something with it
            free(file);
        }
        state_flags.opening_file = false;
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

    // Rendering
    ImGui::Render();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);

    if(state_flags.done) {
        return loop_return_code_t::LOOP_FINISH;
    }
    else {
        return loop_return_code_t::LOOP_CONTINUE;
    }
}

void GUI::buildUI() {
    using namespace ImGui;

    static struct {
        bool register_window = false;
        bool disassemble_window = false;
        bool render_window = true;
    } window_flags;

    if(BeginMainMenuBar()) {
        if(BeginMenu("File")) {
            MenuItem("Open File", NULL, &state_flags.opening_file);
            MenuItem("Enable Debug", NULL, &state_flags.set_debug_mode);
            MenuItem("Exit", NULL, &state_flags.done);
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

                EndMenu();
            }
        }
        EndMainMenuBar();
    }

    if(window_flags.register_window) buildRegisterUI();
    if(window_flags.disassemble_window) buildDisassemblyUI();
    if(window_flags.render_window) buildRenderUI();
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
            Text("CY");
            Text("H");
            Text("N");
            Text("ZF");
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
    Text("0x0000");
    if(reg_flags.AF) {
        Text("0x00");
        Text("0x00");
        if(reg_flags.F) {
            Text("1");
            Text("1");
            Text("1");
            Text("1");
        }
    }

    Text("0x0000");
    if(reg_flags.BC) {
        Text("0x00");
        Text("0x00");
    }

    Text("0x0000");
    if(reg_flags.DE) {
        Text("0x00");
        Text("0x00");
    }

    Text("0x0000");
    if(reg_flags.HL) {
        Text("0x00");
        Text("0x00");
    }

    Text("0x0000");
    Text("0x0000");
    End();
}

void GUI::buildDisassemblyUI() {
    using namespace ImGui;

    SetNextWindowSize({120,0});
    Begin("Disassembly", nullptr, ImGuiWindowFlags_NoResize);
    End();
}