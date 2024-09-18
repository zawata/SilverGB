#include <Foundation/Foundation.h>
#include <GameController/GameController.h>
#include <cmath>
#include <list>
#include <memory>
#include <string>

#import "GameController/GameController.h"
#include "cocoa/util.hh"

#include "binding.hpp"
#include "gamepad.hpp"
#include "util/util.hpp"

bool closeEquals(float a, float b, float buf = 0.01f) {
    return bounded(a, b-buf, b+buf);
}

Silver::Binding::HatDirection convertGCDirection(float x, float y) {
    if (closeEquals(x, 0.0f) && closeEquals(y, 1.0f)) {
        return Silver::Binding::HatDirection::North;
    } else if (closeEquals(x, 1.0f) && closeEquals(y, 1.0f)) {
        return Silver::Binding::HatDirection::NorthEast;
    } else if (closeEquals(x, 1.0f) && closeEquals(y, 0)) {
        return Silver::Binding::HatDirection::East;
    } else if (closeEquals(x, 1.0f) && closeEquals(y, -1.0f)) {
        return Silver::Binding::HatDirection::SouthEast;
    } else if (closeEquals(x, 0) && closeEquals(y, -1.0f)) {
        return Silver::Binding::HatDirection::South;
    } else if (closeEquals(x, -1.0f) && closeEquals(y, -1.0f)) {
        return Silver::Binding::HatDirection::SouthWest;
    } else if (closeEquals(x, -1.0f) && closeEquals(y, 0)) {
        return Silver::Binding::HatDirection::West;
    } else if (closeEquals(x, -1.0f) && closeEquals(y, 1.0f)) {
        return Silver::Binding::HatDirection::NorthWest;
    } else {
        return Silver::Binding::HatDirection::Center; 
    }
}

Silver::Binding::AxisDirection convertGCAxisValue(s16 sdlValue) {
    // TODO: make configurable
    const u16 axisDeadZone = 0.4f;// 40%

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

struct CocoaGamepad : virtual public Silver::Gamepad {
    explicit CocoaGamepad(GCController *controller) : controller(controller) {}

    ~CocoaGamepad() {
    
    }

    void getName(std::string &str) override {
        str = ns_to_s(this->controller.vendorName);
    }

    u16 getVendorId() override {
        return 0;
    }

    u16 getProductId() override {
        return 0;
    }

    int getUniqueId() override {
        return this->controller.hash;
    }

    u8 getButtonCount() override {
        return 0;
    };

    u8 getButton(int idx) override {
        if(idx >= getButtonCount()) {
            return 0;
        }

        return 0;
    };

    u8 getAxisCount() override {
        return 2;
    };

    s16 getAxis(int idx) override {
        if(idx >= getAxisCount()) {
            return 0;
        }

        return 0;
    };

    u8 getHatCount() override {
        // TODO: I don't think apple supports more than 1 dpad per controller?
        return 1;
    };

    Silver::Binding::HatDirection getHat(int idx) override {
        // TODO: I don't think apple supports more than 1 dpad per controller?
        if(idx >= getHatCount()) {
            return Silver::Binding::HatDirection::Invalid;
        }

        return convertGCDirection(
            controller.extendedGamepad.dpad.xAxis.value,
            controller.extendedGamepad.dpad.yAxis.value);
    };

private:
    GCController *controller;
    std::vector<bool> buttonStates;
    std::vector<Silver::Binding::HatDirection> hatStates;
    std::vector<u16> axisStates;
};

Silver::GamepadManager::GamepadManager() {
    NSArray<GCController *> *controllers = GCController.controllers;

    for (int i = 0; i < controllers.count; i++) {
        this->devices.emplace_back(std::make_shared<CocoaGamepad>(controllers[i]));
    }
}

Silver::GamepadManager::~GamepadManager() = default;

void Silver::GamepadManager::getAvailableGamepads(std::vector<std::shared_ptr<Silver::Gamepad>> &vec) {
    std::copy(this->devices.begin(), this->devices.end(), vec.begin());
}

void Silver::GamepadManager::updateGamepads(const std::shared_ptr<Silver::Binding::Tracker>& binding) {
    for (auto &dev : this->devices) {

    }
}

void Silver::GamepadManager::addDevice(int uniqueId) {
    std::erase_if(this->devices, [uniqueId](const std::shared_ptr<Gamepad>& gamepad) -> bool {
        return gamepad->getUniqueId() == uniqueId;
    });
}


void Silver::GamepadManager::removeDevice(int uniqueId) {
    std::erase_if(this->devices, [uniqueId](const std::shared_ptr<Gamepad>& gamepad) -> bool {
        return gamepad->getUniqueId() == uniqueId;
    });
}
