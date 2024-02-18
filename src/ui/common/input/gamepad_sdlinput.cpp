#include <cmath>
#include <list>
#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "binding.hpp"
#include "gamepad.hpp"

Silver::Binding::HatDirection convertSDLDirection(u8 sdlDirection) {
    switch(sdlDirection) {
    case SDL_HAT_CENTERED: return Silver::Binding::HatDirection::Center;
    case SDL_HAT_UP: return Silver::Binding::HatDirection::North;
    case SDL_HAT_RIGHT: return Silver::Binding::HatDirection::East;
    case SDL_HAT_DOWN: return Silver::Binding::HatDirection::South;
    case SDL_HAT_LEFT: return Silver::Binding::HatDirection::West;
    case SDL_HAT_RIGHTUP: return Silver::Binding::HatDirection::NorthEast;
    case SDL_HAT_RIGHTDOWN: return Silver::Binding::HatDirection::SouthEast;
    case SDL_HAT_LEFTUP: return Silver::Binding::HatDirection::NorthWest;
    case SDL_HAT_LEFTDOWN: return Silver::Binding::HatDirection::SouthWest;
    default:return Silver::Binding::HatDirection::Invalid;
    }
}

Silver::Binding::AxisDirection convertSDLAxisValue(s16 sdlValue) {
    // TODO: make configurable
    const u16 axisDeadZone = 13107_u16;// 40%

    Silver::Binding::AxisDirection dir;
    u16 absoluteValue = ::abs(sdlValue);
    if (absoluteValue > axisDeadZone) {
        if (sdlValue > 0) {
            dir = Silver::Binding::AxisDirection::Positive;
        } else {
            dir = Silver::Binding::AxisDirection::Negative;
        }
    } else {
        dir = Silver::Binding::AxisDirection::Center;
    }

    return dir;
}

struct SDLGamepad : virtual public Silver::Gamepad {
    explicit SDLGamepad(SDL_Joystick *joy) : dev(joy) {}

    ~SDLGamepad() {
        SDL_JoystickClose(this->dev);
    }

    void getName(std::string &str) override {
        const char *c = SDL_JoystickName(this->dev);

        str = std::string{c};
    }

    u16 getVendorId() override {
        return SDL_JoystickGetVendor(this->dev);
    }

    u16 getProductId() override {
        return SDL_JoystickGetProduct(this->dev);
    }

    int getUniqueId() override {
        return SDL_JoystickInstanceID(this->dev);
    }

    u8 getButtonCount() override {
        return SDL_JoystickNumButtons(this->dev);
    };

    u8 getButton(int idx) override {
        if(idx >= getButtonCount()) {
            return 0;
        }

        return SDL_JoystickGetAxis(this->dev, idx);
    };

    u8 getAxisCount() override {
        return SDL_JoystickNumAxes(this->dev);
    };

    s16 getAxis(int idx) override {
        if(idx >= getHatCount()) {
            return 0;
        }

        return SDL_JoystickGetAxis(this->dev, idx);
    };

    u8 getHatCount() override {
        return SDL_JoystickNumHats(this->dev);
    };

    Silver::Binding::HatDirection getHat(int idx) override {
        if(idx >= getHatCount()) {
            return Silver::Binding::HatDirection::Invalid;
        }

        u8 direction = SDL_JoystickGetHat(this->dev, idx);
        return convertSDLDirection(direction);
    };

private:
    SDL_Joystick *dev;
    std::vector<bool> buttonStates;
    std::vector<Silver::Binding::HatDirection> hatStates;
    std::vector<u16> axisStates;
};

Silver::GamepadManager::GamepadManager() {
    if(SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cout << "error iniitializing joystick" << SDL_GetError() << std::endl;
        return;
    }

    // SDL_init sets erroneous errors about not finding libudev symbols because it tries to load the symbols in
    // different ways and the first one fails
    SDL_ClearError();

    int numJoysticks = SDL_NumJoysticks();
    if(numJoysticks < 0) {
        std::cerr << "error getting num joysticks: " << SDL_GetError() << std::endl;
        return;
    }

    for(int i = 0; i < numJoysticks; i++) {
        this->addDevice(i);
    }
}

Silver::GamepadManager::~GamepadManager() = default;

void Silver::GamepadManager::getAvailableGamepads(std::vector<std::shared_ptr<Silver::Gamepad>> &vec) {
    std::copy(this->devices.begin(), this->devices.end(), vec.begin());
}

void Silver::GamepadManager::updateGamepads(const std::shared_ptr<Silver::Binding::Tracker>& binding) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_JOYAXISMOTION: {
            Binding::AxisDirection direction = convertSDLAxisValue(event.jaxis.value);

            binding->sendInput(
                    Silver::Binding::makeGamepadAxisInput(
                            event.jaxis.which,
                            event.jaxis.axis,
                            direction),
                    direction != Binding::AxisDirection::Center);
            break;
        }

        case SDL_JOYHATMOTION: {
            auto direction = convertSDLDirection(event.jhat.value);
            if(direction == Binding::HatDirection::Invalid) {
                break;
            }

            binding->sendInput(
                    Silver::Binding::makeGamepadHatInput(
                            event.jhat.which,
                            event.jhat.hat,
                            direction),
                    direction != Binding::HatDirection::Center);
            break;
        }

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP: {
            binding->sendInput(
                    Silver::Binding::makeGamepadButtonInput(
                            event.jbutton.which,
                            event.jbutton.button),
                    event.jbutton.state == SDL_PRESSED);
            break;
        }

        case SDL_JOYDEVICEADDED:
            this->addDevice(event.jdevice.which);
            break;

        case SDL_JOYDEVICEREMOVED:
            this->removeDevice(event.jdevice.which);
            break;

            // case SDL_JOYBALLMOTION:
            // case SDL_JOYBATTERYUPDATED:
        }
    }
}

void Silver::GamepadManager::addDevice(int deviceIndex) {
    SDL_Joystick *joy = SDL_JoystickOpen(deviceIndex);
    if(!joy) {
        throw 1;
    }

    this->devices.emplace_back(std::make_shared<SDLGamepad>(joy));
}

void Silver::GamepadManager::removeDevice(int uniqueId) {
    std::erase_if(this->devices, [uniqueId](const std::shared_ptr<Gamepad>& gamepad) -> bool {
        return gamepad->getUniqueId() == uniqueId;
    });
}
