#include "binding.hpp"

#include <sstream>

#include "input/gamepad.hpp"

const char *buttonStr[] = {"None", "A", "B", "Start", "Select", "Up", "Down", "Left", "Right"};

const char *Silver::Binding::getButtonName(Button button) {
    if(button >= Button::_End) {
        button = Button::None;
    }

    return buttonStr[(int)button];
}

const char *hatDirectionStr[]
        = {"Center", "North", "North East", "East", "South East", "South", "South West", "West", "North West"};

const char *Silver::Binding::getHatDirectionName(Silver::Binding::HatDirection direction) {
    if(direction >= Silver::Binding::HatDirection::_End) {
        return "invalid";
    }

    return hatDirectionStr[(int)direction];
}

const char *axisDirectionStr[] = {"Center", "Positive", "Negative"};

const char *Silver::Binding::getAxisDirectionName(Silver::Binding::AxisDirection direction) {
    if(direction >= Silver::Binding::AxisDirection::_End) {
        return "invalid";
    }

    return axisDirectionStr[(int)direction];
}

std::string Silver::Binding::getInputDescription(const GenericInput &action) {
    std::stringstream ss;
    if(std::holds_alternative<Silver::Binding::KeyboardInput>(action)) {
        auto &keyboardAction = std::get<Silver::Binding::KeyboardInput>(action);
        ss << "Keyboard " << (int)keyboardAction.scanCode;
    } else if(std::holds_alternative<Silver::Binding::GamepadInput>(action)) {
        auto &gamepadAction = std::get<Silver::Binding::GamepadInput>(action);
        ss << "Gamepad " << (int)gamepadAction.deviceIdx << " ";

        if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Axis) {
            ss << "Axis " << gamepadAction.inputIdx << " "
               << Silver::Binding::getAxisDirectionName(static_cast<Binding::AxisDirection>(gamepadAction.direction));
        } else if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Button) {
            ss << "Button " << gamepadAction.inputIdx;
        } else if(gamepadAction.type == Silver::Binding::GamepadInput::Type::Hat) {
            ss << "Hat " << gamepadAction.inputIdx << " "
               << Silver::Binding::getHatDirectionName(static_cast<Binding::HatDirection>(gamepadAction.direction));
        }
    } else {
        ss << "None";
    }

    return ss.str();
}
