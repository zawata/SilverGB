#include <cstring>
#include <sstream>

#include "binding.hpp"
#include "input/gamepad.hpp"

const char *buttonStr[] = {
        "None",
        "A",
        "B",
        "Start",
        "Select",
        "Up",
        "Down",
        "Left",
        "Right"
};

const char *Silver::Binding::getButtonName(Button button) {
    if(button >= Button::_End) {
        button = Button::None;
    }

    return buttonStr[button];
}

std::string Silver::Binding::getInputDescription(const GenericInput &action) {
    std::stringstream ss;
    if(std::holds_alternative<Silver::Binding::KeyboardInput>(action)) {
        auto &keyboardAction = std::get<Silver::Binding::KeyboardInput>(action);
        ss << "Keyboard " << (int) keyboardAction.scanCode;
    } else if(std::holds_alternative<Silver::Binding::GamepadInput>(action)) {
        auto &gamepadAction = std::get<Silver::Binding::GamepadInput>(action);
        ss << "Gamepad " << (int) gamepadAction.deviceIdx << " ";

        if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Axis) {
            ss << "Axis " << gamepadAction.inputIdx << " " << (gamepadAction.direction == -1 ? '-' : '+');
        } else if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Button) {
            ss << "Button " << gamepadAction.inputIdx;
        } else if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Hat) {
            ss << "Hat " << gamepadAction.inputIdx << " "
               << Silver::getHatDirectionName(static_cast<Silver::HatDirection>(gamepadAction.direction));
        }
    } else {
        ss << "None";
    }

    return ss.str();
}
