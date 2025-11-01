#include "gui.hpp"

#include <cmath>

#include "imgui.h"

GUI::GUI(Config *config) :
    config(config) { }

GUI::~GUI() { delete config; }

static void get_screen_area(const ImVec2 &win_bounds, ImVec2 &top_left, ImVec2 &bottom_right) {
    // auto screen sizing and placement code
    float scaling_factor
            = std::fmin(win_bounds.x / Silver::Core::native_width, win_bounds.y / Silver::Core::native_height);
    float width      = scaling_factor * Silver::Core::native_width;
    float height     = scaling_factor * Silver::Core::native_height;
    float wRemainder = win_bounds.x - width;
    float hRemainder = win_bounds.y - height;

    top_left         = {wRemainder / 2.0f, hRemainder / 2.0f};
    bottom_right     = {top_left.x + width, top_left.y + height};
}

void buildScreenView(Silver::Application *app) {
    namespace im          = ImGui;
    bool drawToBackground = true;

    if(app->app_state.debug.enabled) {
        drawToBackground = app->app_state.debug.drawToBackground;
    }

    if(drawToBackground) {
        ImVec2 top_left, bottom_right;
        get_screen_area(ImGui::GetMainViewport()->WorkSize, top_left, bottom_right);
        ImGui::GetBackgroundDrawList()->AddImage((void *)app->screen_texture_id, top_left, bottom_right);
    } else {
        // TODO: set the sive of this better
        if(im::Begin("Game")) {
            ImVec2 size = im::GetWindowSize();
            im::BeginChild("##Render");
            im::Image((void *)app->screen_texture_id, size);
            im::EndChild();
        }

        im::End();
    }
}

void buildFpsWindow(float fps) {
    namespace im = ImGui;

    im::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);
    im::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    im::PushStyleVar(ImGuiStyleVar_WindowMinSize, {20, 20});
    im::Begin(
            "FPS",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                    | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);

    im::Text("%0.1f", fps);

    auto viewport_width = im::GetMainViewport()->Size.x;
    auto window_width   = im::GetWindowWidth();

    im::SetWindowPos({viewport_width - (window_width + 5), 5});

    im::End();
    im::PopStyleVar(3);
}

void buildDebugWindow(Silver::Application *app) {
    namespace im = ImGui;

    if(!im::Begin("Debug", nullptr, 0)) {
        im::End();
        return;
    }

    if(im::BeginTabBar("##TabBar")) {
        if(im::BeginTabItem("Background")) {
            auto maxWidth    = im::GetContentRegionMax();
            auto itemSpacing = im::GetStyle().ItemSpacing.y;
            auto edgeSize    = (maxWidth.x - (itemSpacing * 2));
            im::Image(app->debug_bg_texture_id, {edgeSize, edgeSize});
            im::EndTabItem();
        }

        if(im::BeginTabItem("Tiles")) {
            auto maxWidth    = im::GetContentRegionMax();
            auto itemSpacing = im::GetStyle().ItemSpacing.y;
            for(int i = 0; i < 3; i++) {
                int  row    = i << 1;

                // I think the *3 here is +2 from the outside of the images, and +1 from between them, but I'm not sure
                auto width  = (maxWidth.x - (itemSpacing * 3)) / 2;
                auto height = width / 2;

                im::Image(app->vram_debug_texture_ids[row], {width, height});
                im::SameLine(0, itemSpacing);
                im::Image(app->vram_debug_texture_ids[row + 1], {width, height});
            }
            im::EndTabItem();
        }

        // if(im::BeginTabItem("Window")) {
        //     im::Image((void *) app->debug_wnd_texture_id, {256, 256});
        //     im::EndTabItem();
        // }

        im::EndTabBar();
    }

    im::End();
}

void buildCPURegisterWindow(Silver::Core *core) {
    using namespace ImGui;

    static struct {
        bool AF;
        bool F;
        bool BC;
        bool DE;
        bool HL;
    } reg_flags;

    CPU::registers_t regs {};
    if(core) {
        regs = core->getRegistersFromCPU();
    } else {
        regs = {0};
    }

    // The Indentation on this window is very sensitive...
    SetNextWindowSize({120, 0});
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
            Text("Z");
            Text("N");
            Text("H");
            Text("C");
            Unindent(5.0);
            TreePop();
        } else {
            reg_flags.F = false;
        }
        Indent(15.0);
        TreePop();
    } else {
        reg_flags.F  = false;
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

    // AF
    Text("0x%s", itoh(regs.AF, 4).c_str());
    if(reg_flags.AF) {
        Text("0x%s", itoh(regs.AF >> 8, 2).c_str());   // A
        Text("0x%s", itoh(regs.AF & 0xFF, 2).c_str()); // F
        if(reg_flags.F) {
            Text("%u", Bit::test(regs.AF, 7));         // Z
            Text("%u", Bit::test(regs.AF, 6));         // N
            Text("%u", Bit::test(regs.AF, 5));         // H
            Text("%u", Bit::test(regs.AF, 4));         // C
        }
    }

    // BC
    Text("0x%s", itoh(regs.BC, 4).c_str());
    if(reg_flags.BC) {
        Text("0x%s", itoh(regs.BC >> 8, 2).c_str());   // B
        Text("0x%s", itoh(regs.BC & 0xFF, 2).c_str()); // C
    }

    // DE
    Text("0x%s", itoh(regs.DE, 4).c_str());
    if(reg_flags.DE) {
        Text("0x%s", itoh(regs.DE >> 8, 2).c_str());   // D
        Text("0x%s", itoh(regs.DE & 0xFF, 2).c_str()); // E
    }

    // HL
    Text("0x%s", itoh(regs.HL, 4).c_str());
    if(reg_flags.HL) {
        Text("0x%s", itoh(regs.HL >> 8, 2).c_str());   // H
        Text("0x%s", itoh(regs.HL & 0xFF, 2).c_str()); // L
    }

    // PC
    Text("0x%s", itoh(regs.PC, 4).c_str());

    // SP
    Text("0x%s", itoh(regs.SP, 4).c_str());
    End();
}

void buildIORegisterWindow(Silver::Core *core) {
    using namespace ImGui;

    Memory::io_registers_t regs {};
    if(core) {
        regs = core->getregistersfromIO();
    } else {
        regs = {0};
    }

    // The Indentation on this window is very sensitive...
    SetNextWindowSize({120, 0});
    Begin("IO Registers", nullptr, ImGuiWindowFlags_NoResize);
    Columns(2, nullptr, true);
    SetColumnWidth(-1, 60);

    Text("P1");
    Text("SB");
    Text("SC");
    Text("DIV");
    Text("TIMA");
    Text("TMA");
    Text("TAC");
    Text("IF");
    Text("LCDC");
    Text("STAT");
    Text("SCY");
    Text("SCX");
    Text("LY");
    Text("LYC");
    Text("DMA");
    Text("BGP");
    Text("OBP0");
    Text("OBP1");
    Text("WY");
    Text("WX");
    Text("VBK");
    Text("SVBK");
    Text("IE");
    NextColumn();
    SetColumnWidth(-1, 50);

    Text("0x%s", itoh(regs.P1, 2).c_str());
    Text("0x%s", itoh(regs.SB, 2).c_str());
    Text("0x%s", itoh(regs.SC, 2).c_str());
    Text("0x%s", itoh(regs.DIV, 2).c_str());
    Text("0x%s", itoh(regs.TIMA, 2).c_str());
    Text("0x%s", itoh(regs.TMA, 2).c_str());
    Text("0x%s", itoh(regs.TAC, 2).c_str());
    Text("0x%s", itoh(regs.IF, 2).c_str());
    Text("0x%s", itoh(regs.LCDC, 2).c_str());
    Text("0x%s", itoh(regs.STAT, 2).c_str());
    Text("0x%s", itoh(regs.SCY, 2).c_str());
    Text("0x%s", itoh(regs.SCX, 2).c_str());
    Text("0x%s", itoh(regs.LY, 2).c_str());
    Text("0x%s", itoh(regs.LYC, 2).c_str());
    Text("0x%s", itoh(regs.DMA, 2).c_str());
    Text("0x%s", itoh(regs.BGP, 2).c_str());
    Text("0x%s", itoh(regs.OBP0, 2).c_str());
    Text("0x%s", itoh(regs.OBP1, 2).c_str());
    Text("0x%s", itoh(regs.WY, 2).c_str());
    Text("0x%s", itoh(regs.WX, 2).c_str());
    Text("0x%s", itoh(regs.VBK, 2).c_str());
    Text("0x%s", itoh(regs.SVBK, 2).c_str());
    Text("0x%s", itoh(regs.IE, 2).c_str());
    End();
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
//                             memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2,
//                             true).c_str(), 3);
//                             *++c_index = ' ';
//                             c_index++;
//                         }
//                     } else {
//                         for(int i = start_floor_diff; i < end_int; i++) {
//                             memcpy(c_index++, itoh(core->getByteFromIO(start_int - start_floor_diff + i), 2,
//                             true).c_str(), 3);
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

// TODO: fix
void buildBreakpointWindow(Silver::Core *core) {
    using namespace ImGui;

    ImGuiInputTextCallback text_func = [](ImGuiInputTextCallbackData *data) -> int {
        switch(data->EventChar) {
        case '0' ... '9': break;
        case 'a' ... 'f': data->EventChar -= ('a' - 'A'); break;
        case 'A' ... 'F': break;
        default:          return 1;
        }

        return 0;
    };

    static char buf[5] = "";

    if(Begin("Breakpoint", nullptr, ImGuiWindowFlags_NoResize)) {
        Columns(2, nullptr, true);
        SetColumnWidth(-1, 90);
        Dummy({0, 0.25});
        Text("Breakpoint:");
        Dummy({0, 0.25});
        Text("Enabled:");
        NextColumn();
        PushItemWidth(50);
        InputText("##", buf, 5, ImGuiInputTextFlags_CallbackCharFilter, text_func);
        PopItemWidth();

        bool breakpoint_enabled = false;
        if(core) {
            breakpoint_enabled = core->get_bp_active();
        }

        if(Checkbox("##", &breakpoint_enabled) && core) {
            core->set_bp_active(breakpoint_enabled);
        }
    }
    End();
}
