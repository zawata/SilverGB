#pragma once

#include <cassert>
#include <vector>

#include <SDL2/SDL.h>

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
 * checks its shortcut in an `if` statement.
 * not very complex but effective.
 *
 * This does pose issues with using keyboard events and shortcuts simultaneously as we
 * might accidentally attempt to use keyboard events as shortcuts unintentionally.
 *
 * We can overcome this by checking if a keyevent has a modifier on it and denying all
 * shortcut requests without a modifier key.
 **/

namespace ImGui {

typedef struct __shortcut {
    bool valid; //actually initialized

    bool ctrl_mod;
    bool alt_mod;
    bool shift_mod;
    char key;
} shortcut_t;

typedef enum {
    NO_MOD = 0,
    CTRL_MOD = 1,
    ALT_MOD,
    SHIFT_MOD,
} key_mods;

static shortcut_t current_shortcut;

void ClearShortcut() {
    current_shortcut = {
        .valid = false,
    };
}

bool BuildShortcut(SDL_KeyboardEvent *key) {
    ClearShortcut();
    if(     key->keysym.sym >= 32 &&  //if ascii
            key->keysym.sym <= 122 &&
            key->keysym.mod &&        //if it has a modifier
            key->state == SDL_PRESSED) { //if key was pressed
        if (key->keysym.mod & KMOD_CTRL ||
            key->keysym.mod & KMOD_ALT ||
            key->keysym.mod & KMOD_SHIFT) {
            current_shortcut = {
                .valid = true,
                .ctrl_mod = (bool)(key->keysym.mod & KMOD_CTRL),
                .alt_mod = (bool)(key->keysym.mod & KMOD_ALT),
                .shift_mod = (bool)(key->keysym.mod & KMOD_SHIFT),
                .key = (char)(key->keysym.sym),
            };
            return true;
        }
    }
    return false;
}

bool ShortcutPressed(std::vector<key_mods> mods, char key) {
    assert(mods.size() <= 3);
    assert(mods[0] != 0);
    if(!current_shortcut.valid) return false;

    bool ctrl_repeated = false,
         alt_repeated = false,
         shift_repeated = false;

    for(int i = 0; i < mods.size(); i++) {
        switch(mods[i]) {
        case 0:
            break;
        case CTRL_MOD:
            assert(!ctrl_repeated);
            if(!current_shortcut.ctrl_mod) return false;
            ctrl_repeated = true;
            break;
        case ALT_MOD:
            assert(!alt_repeated);
            if(!current_shortcut.alt_mod) return false;
            alt_repeated = true;
            break;
        case SHIFT_MOD:
            assert(!shift_repeated);
            if(!current_shortcut.shift_mod) return false;
            shift_repeated = true;
            break;
        }
    }
    return key            == current_shortcut.key &&
           ctrl_repeated  == current_shortcut.ctrl_mod &&
           alt_repeated   == current_shortcut.alt_mod &&
           shift_repeated == current_shortcut.shift_mod;
}

}