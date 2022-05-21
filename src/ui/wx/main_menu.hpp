#pragma once

#include <memory>

#include <wx/filename.h>
#include <wx/app.h>
#include <wx/event.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/version.h>

#include "cfg/cfg.hpp"

enum {
    #define wxID_next() (wxID_HIGHEST + __LINE__)

    id_File_LoadROM = wxID_next(),
    id_File_Recents_NoRecentFiles = wxID_next(),

    id_File_Recents_FileIdx0 = wxID_next(),
    id_File_Recents_FileIdx1 = wxID_next(),
    id_File_Recents_FileIdx2 = wxID_next(),
    id_File_Recents_FileIdx3 = wxID_next(),
    id_File_Recents_FileIdx4 = wxID_next(),
    id_File_Recents_FileIdx5 = wxID_next(),
    id_File_Recents_FileIdx6 = wxID_next(),
    id_File_Recents_FileIdx7 = wxID_next(),
    id_File_Recents_FileIdx8 = wxID_next(),
    id_File_Recents_FileIdx9 = wxID_next(),

    id_File_Quit = wxID_EXIT,

    id_Emulation_SetBIOS = wxID_next(),
    id_Emulation_Device_Gameboy = wxID_next(),
    id_Emulation_Device_Super_Gameboy = wxID_next(),
    id_Emulation_Device_Gameboy_Color = wxID_next(),
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

    #undef wxID_next
};


class MenuManager {
    std::shared_ptr<Config> config;

    static constexpr const char * file_label = "File";
    wxMenu *generateFileMenu() {
        wxMenu *fileMenu = new wxMenu(); {
            fileMenu->Append(id_File_LoadROM,  "Load Rom\tCtrl-F");
            wxMenu *fileRecentsMenu = new wxMenu(); {
                if(config->fileSettings.recent_files.empty()) {
                    fileRecentsMenu->Append(id_File_Recents_NoRecentFiles, "No Recent Files");
                    fileRecentsMenu->Enable(id_File_Recents_NoRecentFiles, false);
                } else {
                    int cntr = 0;
                    for(auto const& file : config->fileSettings.recent_files) {
                        fileRecentsMenu->Append(id_File_Recents_FileIdx0 + cntr++, wxFileName(file).GetFullName(), file);
                    }
                }
            }
            fileMenu->AppendSubMenu(fileRecentsMenu, "Recent Files");

            fileMenu->Append(id_File_Quit, "Exit\tCtrl-W");
        }

        return fileMenu;
    }

    static constexpr const char * emulation_label = "Emulation";
    wxMenu *generateEmulationMenu() {
        wxMenu *emuMenu = new wxMenu(); {
            emuMenu->AppendCheckItem(id_Emulation_SetBIOS, "Set BIOS");

            wxMenu *deviceMenu = new wxMenu(); {
                deviceMenu->AppendRadioItem(id_Emulation_Device_Gameboy,         "Gameboy");
                deviceMenu->AppendRadioItem(id_Emulation_Device_Super_Gameboy,   "Super Gameboy");
                deviceMenu->AppendRadioItem(id_Emulation_Device_Gameboy_Color,   "Gameboy Color");

                deviceMenu->Check(id_Emulation_Device_Gameboy, true);
            }
            emuMenu->AppendSubMenu(deviceMenu, "Emulated Device", "Set Emulated Device");

            wxMenu *execModeMenu = new wxMenu(); {
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_Normal,    "Normal");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepClk,   "Step Clock Cycle");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepInstr, "Step Instruction");
                execModeMenu->AppendRadioItem(id_Emulation_ExecMode_StepFrame, "Step Frame");

                execModeMenu->Check(id_Emulation_ExecMode_Normal, true);
            }
            emuMenu->AppendSubMenu(execModeMenu, "Execution Mode", "Set Execution Mode");

            emuMenu->Append(id_Emulation_step, "Step\tCtrl-T", "Step According to Execution Mode");

            if(!config->emulationSettings.bios_file.empty()) {
                emuMenu->Check(id_Emulation_SetBIOS, true);
                emuMenu->Enable(id_Emulation_SetBIOS, false);
            }
        }

        return emuMenu;
    }

    static constexpr const char * view_label = "View";
    wxMenu *generateViewMenu() {
        wxMenu *viewMenu = new wxMenu(); {
            wxMenu *dispModeMenu = new wxMenu(); {
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Fit,     "Fit");
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Stretch, "Stretch");
                dispModeMenu->AppendRadioItem(id_View_DisplayMode_Center,  "Fixed");

                dispModeMenu->Check(id_View_DisplayMode_Fit, true);
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

                auto size = config->viewSettings.size;
                sizeMenu->Check(id_View_Size_1x1 + size - 1, true);
            }
            viewMenu->AppendSubMenu(sizeMenu, "Size");
        }

        return viewMenu;
    }

    static constexpr const char * help_label = "Help";
    wxMenu *generateHelpMenu() {
        wxMenu *helpMenu = new wxMenu(); {
            helpMenu->Append(id_Help_About, "About\tF1", "Show about dialog");
        }

        return helpMenu;
    }

public:
    MenuManager(std::shared_ptr<Config> config): config(config) {}

    void regenerateFileMenu(wxMenuBar *menuBar) {
        int idx = menuBar->FindMenu(file_label);

        if(idx != wxNOT_FOUND) {
            wxMenu *menu = menuBar->Replace(idx, generateFileMenu(), file_label);
            delete menu;
            return;
        }

        assert(false);
    }

    void regenerateEmulationMenu(wxMenuBar *menuBar) {
        int idx = menuBar->FindMenu(emulation_label);

        if(idx != wxNOT_FOUND) {
            wxMenu *menu = menuBar->Replace(idx, generateEmulationMenu(), emulation_label);
            delete menu;
            return;
        }

        assert(false);
    }

    void regenerateViewMenu(wxMenuBar *menuBar) {
        int idx = menuBar->FindMenu(view_label);

        if(idx != wxNOT_FOUND) {
            wxMenu *menu = menuBar->Replace(idx, generateViewMenu(), view_label);
            delete menu;
            return;
        }

        assert(false);
    }

    void regenerateHelpMenu(wxMenuBar *menuBar) {
        int idx = menuBar->FindMenu(help_label);

        if(idx != wxNOT_FOUND) {
            wxMenu *menu = menuBar->Replace(idx, generateHelpMenu(), help_label);
            delete menu;
            return;
        }

        assert(false);
    }

    wxMenuBar *generateMenuBar() {
        wxMenuBar *menuBar = new wxMenuBar();

        menuBar->Append(generateFileMenu(), "File");
        menuBar->Append(generateEmulationMenu(), "Emulation");
        menuBar->Append(generateViewMenu(), "View");
        menuBar->Append(generateHelpMenu(), "Help");

        return menuBar;
    }
};