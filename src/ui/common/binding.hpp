#pragma once

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "gb_core/joy.hpp"

namespace Silver {
  namespace Binding {
    enum Button {
      None = 0,
      A,
      B,
      Start,
      Select,
      Up,
      Down,
      Left,
      Right,
      _End
    };

    template<typename kT>
    struct Tracker {
      std::unordered_map<kT, Button> key_map{};
      std::vector<bool> button_state{};
      Button recording_for_button = Button::None;

      Tracker() {
        button_state.reserve(Button::_End);
      }

      void resetButtons() {
        std::fill(button_state.begin(), button_state.end(),false);
      }

      void printKeyMap() {
        std::cout
            << "A:      " << getKeyForButton(Button::A) << std::endl
            << "B:      " << getKeyForButton(Button::B) << std::endl
            << "Start:  " << getKeyForButton(Button::Start) << std::endl
            << "Select: " << getKeyForButton(Button::Select) << std::endl
            << "Up:     " << getKeyForButton(Button::Up) << std::endl
            << "Down:   " << getKeyForButton(Button::Down) << std::endl
            << "Left:   " << getKeyForButton(Button::Left) << std::endl
            << "Right:  " << getKeyForButton(Button::Right) << std::endl;
      }

      void startRecording(Button button) {
        recording_for_button = button;
        kT key = getKeyForButton(button);
        if (key) {
          mapButton(key, Button::None);
        }
      }

      bool isRecording() {
        return recording_for_button != Button::None;
      }

      void mapButton(kT key, Button button) {
        key_map[key] = button;
      }

      bool isKeyMapped(kT key) {
        return key_map.find(key) != key_map.end();
      }

      Button getButtonForKey(kT key) {
        try {
          return key_map.at(key);
        } catch (const std::out_of_range &e) {
          return Button::None;
        }
      }

      kT getKeyForButton(Button button) {
        for (const auto& node : key_map) {
          if (node.second == button) {
            return node.first;
          }
        }

        return 0;
      }

      void setStateForKey(kT key, bool pressed = true) {
        Button button = Button::None;
        if (isRecording()) {
          button = recording_for_button;
          recording_for_button = Button::None;

          mapButton(key, button);
        } else {
          button = getButtonForKey(key);
        }

        assert(button != Button::None);

        setStateForButton(button, pressed);
      }

      void setStateForButton(Button button, bool pressed = true) {
        if (button != Button::None) {
          button_state[button] = pressed;
        }
      }

      bool isButtonPressed(Button button) {
        return button_state[button];
      }

      void getButtonStates(Joypad::button_states_t &button_states) {
          button_states.a = this->button_state[Button::A];
          button_states.b = this->button_state[Button::B];
          button_states.start = this->button_state[Button::Start];
          button_states.select = this->button_state[Button::Select];
          button_states.up = this->button_state[Button::Up];
          button_states.down = this->button_state[Button::Down];
          button_states.left = this->button_state[Button::Left];
          button_states.right = this->button_state[Button::Right];
      }
    };
  }
}