#pragma once

#include <memory>
#include <list>

#include "binding.hpp"
#include "util/types/primitives.hpp"

namespace Silver {
    struct GamepadManager;

    extern const char *getHatDirectionName(Binding::HatDirection direction);

    struct Gamepad {
#if defined(USE_MACOS_GAMECONTROLLER)
        using UniqueId = void *;
#elif defined(USE_SDLINPUT)
        using UniqueId = int;
#else
        #error "must define a gamepad implementation"
#endif

        virtual void getName(std::string &str) = 0;
        virtual u16 getVendorId() = 0;
        virtual u16 getProductId() = 0;
        virtual UniqueId getUniqueId() = 0;

        virtual u8 getButtonCount() = 0;
        virtual u8 getButton(int idx) = 0;
        virtual u8 getAxisCount() = 0;
        virtual s16 getAxis(int idx) = 0;
        virtual u8 getHatCount() = 0;
        virtual Binding::HatDirection getHat(int idx) = 0;

    private:
        friend GamepadManager;
    };

    struct GamepadManager {
        GamepadManager();
        ~GamepadManager();

        void getAvailableGamepads(std::vector<std::shared_ptr<Gamepad>> &vec);
        void updateGamepads(const std::shared_ptr<Binding::Tracker>& binding);

    protected:
        void addDevice(Gamepad::UniqueId uniqueId);
        void removeDevice(Gamepad::UniqueId uniqueId);

    private:
        std::list<std::shared_ptr<Gamepad>> devices{};
    };
}