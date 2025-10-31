#pragma once

#include <cassert>
#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

#include "gb_core/joy.hpp"

namespace Silver::Binding {
    struct KeyboardInput {
        int  scanCode;

        bool operator== (const KeyboardInput &other) const { return scanCode == other.scanCode; }
    };

    struct GamepadInput {
        enum Type { Button, Axis, Hat };

        Type type;

        int  deviceIdx;
        int  inputIdx;
        int  direction;

        bool operator== (const GamepadInput &other) const {
            return type == other.type && deviceIdx == other.deviceIdx && inputIdx == other.inputIdx;
        }
    };

    using GenericInput = std::variant<KeyboardInput, GamepadInput>;
} // namespace Silver::Binding

// these must exist outside any namespaces, before their usage in a hashtable, and after the struct definitions
template<>
struct std::hash<Silver::Binding::KeyboardInput> {
    std::size_t operator() (const Silver::Binding::KeyboardInput &k) const { return hash<int>()(k.scanCode); }
};

template<>
struct std::hash<Silver::Binding::GamepadInput> {
    std::size_t operator() (const Silver::Binding::GamepadInput &k) const {
        auto type  = hash<int>()(k.type);
        auto dev   = hash<int>()(k.deviceIdx);
        auto input = hash<int>()(k.inputIdx);

        if(type != Silver::Binding::GamepadInput::Button) {
            input <<= 8;
            input |= k.direction;
        }

        return (type << 32) | (dev << 16) | input;
    }
};

namespace Silver::Binding {
    enum class Button { None = 0, A, B, Start, Select, Up, Down, Left, Right, _End };

    enum class HatDirection {
        Invalid = -1,
        Center,
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest,
        _End
    };

    enum class AxisDirection { Invalid = -1, Center, Positive, Negative, _End };

    extern const char  *getButtonName(Button button);
    extern const char  *getHatDirectionName(HatDirection);
    extern const char  *getAxisDirectionName(AxisDirection);

    std::string         getInputDescription(const GenericInput &input);

    inline GenericInput makeKeyboardInput(int scanCode) { return GenericInput {KeyboardInput {scanCode}}; }

    inline GenericInput makeGamepadButtonInput(int gamepadIdx, int buttonIndex) {
        return GenericInput {
            GamepadInput {GamepadInput::Type::Button, gamepadIdx, buttonIndex}
        };
    }

    inline GenericInput makeGamepadAxisInput(int gamepadIdx, int axisIndex, AxisDirection axisDirection) {
        return GenericInput {
            GamepadInput {GamepadInput::Type::Axis, gamepadIdx, axisIndex, (int)axisDirection}
        };
    }

    inline GenericInput makeGamepadHatInput(int gamepadIdx, int hatIndex, HatDirection hatDirection) {
        return GenericInput {
            GamepadInput {GamepadInput::Type::Hat, gamepadIdx, hatIndex, (int)hatDirection}
        };
    }

    struct Tracker {
        Tracker() { button_state.resize((int)Button::_End); }

        void clear() { std::fill(button_state.begin(), button_state.end(), false); }

        void startRecording(Button button) {
            recording_for_button = button;
            start_recording_time = std::chrono::steady_clock::now();
        }

        void stopRecording() {
            recording_for_button = Button::None;
            start_recording_time.reset();
        }

        bool isRecording() { return !isRecordingForButton(Button::None); }

        bool isRecordingForButton(Button button) { return recording_for_button == button; }

        std::optional<std::chrono::steady_clock::duration> time_since_recording_started() {
            if(!this->isRecording() || !this->start_recording_time.has_value()) {
                return {};
            }

            return {std::chrono::steady_clock::now() - this->start_recording_time.value()};
        }

        void   mapButton(GenericInput input, Button button) { input_map[input] = button; }

        bool   isInputMapped(GenericInput input) { return input_map.find(input) != input_map.end(); }

        Button getButtonForInput(GenericInput input) {
            try {
                return input_map.at(input);
            } catch(const std::out_of_range &e) {
                return Button::None;
            }
        }

        std::optional<GenericInput> getInputForButton(Button button) {
            for(const auto &node : input_map) {
                if(node.second == button) {
                    return node.first;
                }
            }

            return {};
        }

        bool sendInput(GenericInput input, bool pressed) {
            Button button = Button::None;
            if(isRecording()) {
                if(!pressed) {
                    return false;
                }

                mapButton(input, recording_for_button);

                auto nextButton = static_cast<Button>((int)recording_for_button + 1);
                if(nextButton < Button::_End) {
                    startRecording(nextButton);
                } else {
                    stopRecording();
                }

                return true;
            }

            button = getButtonForInput(input);
            if(button != Button::None) {
                button_state[(int)button] = pressed;
                return true;
            }

            return false;
        }

        bool isButtonPressed(Button button) { return button_state[(int)button]; }

        void getButtonStates(Joypad::button_states_t &button_states) {
            button_states.a      = isButtonPressed(Button::A);
            button_states.b      = isButtonPressed(Button::B);
            button_states.start  = isButtonPressed(Button::Start);
            button_states.select = isButtonPressed(Button::Select);
            button_states.up     = isButtonPressed(Button::Up);
            button_states.down   = isButtonPressed(Button::Down);
            button_states.left   = isButtonPressed(Button::Left);
            button_states.right  = isButtonPressed(Button::Right);
        }

    private:
        std::unordered_map<GenericInput, Button>             input_map {};
        std::vector<bool>                                    button_state {};
        Button                                               recording_for_button = Button::None;
        std::optional<std::chrono::steady_clock::time_point> start_recording_time;
    };
} // namespace Silver::Binding
