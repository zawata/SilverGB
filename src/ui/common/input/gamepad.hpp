#pragma once

#include <memory>
#include <list>

#include "binding.hpp"
#include "util/types/primitives.hpp"

namespace Silver {
    struct GamepadManager;

    enum HatDirection {
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

    extern const char *getHatDirectionName(Silver::HatDirection direction);

    struct Gamepad {
        virtual void getName(std::string &str) = 0;
        virtual u16 getVendorId() = 0;
        virtual u16 getProductId() = 0;
        virtual int getUniqueId() = 0;

        virtual u8 getButtonCount() = 0;
        virtual u8 getButton(int idx) = 0;
        virtual u8 getAxisCount() = 0;
        virtual s16 getAxis(int idx) = 0;
        virtual u8 getHatCount() = 0;
        virtual Silver::HatDirection getHat(int idx) = 0;
    private:
        friend GamepadManager;
    };

    struct GamepadManager {
        GamepadManager();
        ~GamepadManager();

        void getAvailableGamepads(std::vector<std::shared_ptr<Gamepad>> &vec);
        void updateGamepads(const std::shared_ptr<Silver::Binding::Tracker>& binding);

    protected:
        void addDevice(int uniqueId);
        void removeDevice(int uniqueId);

    private:
        std::list<std::shared_ptr<Gamepad>> devices{};
    };
}