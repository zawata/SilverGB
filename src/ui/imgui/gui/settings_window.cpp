
#include "settings_window.hpp"

#include "../app.hpp"
#include "imgui/imgui.h"

const char *settings_titles[] = {"Emulation", "Display", "Audio", "Input", "Debug", "About"};
auto        settings_order    = std::array<SettingsSection, 6> {Emulation, Display, Audio, Input, Debug, About};

void        buildEmulationSettingsSection(Silver::Application *app);
void        buildDisplaySettingsSection(Silver::Application *app);
void        buildAudioSettingsSection(Silver::Application *app);
void        buildInputSettingsSection(Silver::Application *app);
void        buildDebugSettingsSection(Silver::Application *app);
void        buildAboutSettingsSection(Silver::Application *app);

void        buildSettingsWindow(Silver::Application *app) {
    namespace im                           = ImGui;

    static SettingsSection selectedSection = SettingsSection::Emulation;

    im::SetNextWindowSize(im::GetMainViewport()->Size);
    im::SetNextWindowPos({0, 0});
    im::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);
    im::Begin(
            "Settings",
            nullptr,
            0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                    | ImGuiWindowFlags_NoCollapse);

    if(ImGui::BeginTable("table1", 2)) {
        ImGui::TableNextRow();
        im::TableSetColumnIndex(0);

        if(ImGui::BeginListBox("listbox 1")) {
            for(auto const &section : settings_order) {
                const bool is_selected = (selectedSection == section);
                if(ImGui::Selectable(settings_titles[(int)section], is_selected)) {
                    selectedSection = section;
                }
            }

            ImGui::EndListBox();
        }

        im::TableSetColumnIndex(1);

        ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.75f, 260), ImGuiChildFlags_None);
        im::Text("%s Settings", settings_titles[(int)selectedSection]);
        switch(selectedSection) {
        case SettingsSection::Emulation: buildEmulationSettingsSection(app); break;
        case SettingsSection::Display:   buildDisplaySettingsSection(app); break;
        case SettingsSection::Audio:     buildAudioSettingsSection(app); break;
        case SettingsSection::Input:     buildInputSettingsSection(app); break;
        case SettingsSection::Debug:     buildDebugSettingsSection(app); break;
        case SettingsSection::About:     buildAboutSettingsSection(app); break;
        }
        ImGui::EndChild();

        ImGui::EndTable();
    }

    im::End();
    im::PopStyleVar();
}

void buildEmulationSettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    im::Checkbox("Enable Frame Skip", &app->config->emu.enable_frame_skip);
    im::SliderInt("Frame Skip", &app->config->emu.frame_skip, 0, 10);
}

void buildDisplaySettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    // im::Checkbox("Enable VSync", &app->config->display.enable_vsync);
}

void buildAudioSettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    im::Checkbox("Enable Audio", &app->config->audio.enable);
    im::SliderInt("Audio Volume", &app->config->audio.volume, 0, 100);
}

void buildInputSettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    if(im::BeginTable("inputs", 3, ImGuiTableFlags_NoHostExtendX)) {
        im::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, 30);
        im::TableSetupColumn("Input");
        im::TableSetupColumn("Clear");

        for(int button = (int)Silver::Binding::Button::A; button < (int)Silver::Binding::Button::_End; button++) {
            im::TableNextRow();
            // Button Name
            im::TableSetColumnIndex(0);
            im::Text("%s", Silver::Binding::getButtonName((Silver::Binding::Button)button));

            // Button mapping
            im::TableSetColumnIndex(1);

            bool        isRecording = app->binding->isRecordingForButton((Silver::Binding::Button)button);

            auto        maybeAction = app->binding->getInputForButton((Silver::Binding::Button)button);
            std::string button_label;
            if(isRecording) {
                button_label = "Recording";
            } else if(maybeAction.has_value()) {
                button_label = Silver::Binding::getInputDescription(maybeAction.value());
            } else {
                button_label = "Unmapped";
            }

            if(im::Button(button_label.c_str())) {
                app->binding->startRecording((Silver::Binding::Button)button);
            }
        }

        im::EndTable();
    }
}

void buildDebugSettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    im::Checkbox("Enable Debug Mode", &app->config->debug.enable_debug_mode);
}

void buildAboutSettingsSection(Silver::Application *app) {
    namespace im = ImGui;

    im::Text("SilverGB");
}
