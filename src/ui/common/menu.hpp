#pragma once

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <utility>

#define ID_COUNTER_START 40500

namespace Silver {
    struct Menu;

    struct [[maybe_unused]] MenuItem {
        enum Type {
            None,
            Text,
            SubMenu,
            Separator,
            Callback,
            Toggle,
            Radio
        };

        virtual ~MenuItem() = default;

        virtual bool operator==(MenuItem &n) const { return get_id() == n.get_id(); }
        virtual bool operator!=(MenuItem &n) const { return !operator==(n); }

        [[nodiscard]] int get_id() const { return item_id; }
        [[nodiscard]] virtual MenuItem::Type get_type() const {
            return MenuItem::Type::None;
        }

    protected:
        MenuItem() :
        item_id(_new_id()) {
        }

    private:
        static int _new_id() {
            static int id = ID_COUNTER_START;
            return id++;
        }

        int item_id;
    };

    struct [[maybe_unused]] TextMenuItem : public MenuItem {
        explicit TextMenuItem(std::string label) :
                label(std::move(label)) {}

        explicit TextMenuItem(const char *label) :
                label(label) {}

        virtual MenuItem::Type get_type() const override {
            return MenuItem::Type::Text;
        }

        bool operator==(MenuItem &n) const override {
            if(n.get_type() == MenuItem::Type::Text) {
                return dynamic_cast<TextMenuItem &>(n).label == this->label;
            }

            return false;
         }

        const std::string label;
    };

    struct [[maybe_unused]] SeparatorMenuItem : public MenuItem {
        SeparatorMenuItem() = default;

        virtual MenuItem::Type get_type() const override {
            return MenuItem::Type::Separator;
        }
    };

    struct [[maybe_unused]] ToggleMenuItem : public TextMenuItem {
        using CallbackType = std::function<void(const ToggleMenuItem &, bool, void *)>;

        ToggleMenuItem(std::string label, CallbackType cb, void *us_data = nullptr, bool initial_state = false) :
                TextMenuItem(std::move(label)),
                cb(std::move(cb)),
                callback_mode(true),
                state_toggle(nullptr),
                default_state(initial_state),
                us_data(us_data) {}

        ToggleMenuItem(const char *label, CallbackType cb, void *us_data = nullptr, bool initial_state = false) :
                TextMenuItem(label),
                cb(std::move(cb)),
                callback_mode(true),
                state_toggle(nullptr),
                default_state(initial_state),
                us_data(us_data) {}

        void operator()(bool new_state) {
            if(callback_mode)
                cb(*this, new_state, us_data);
            else {
                if(state_toggle)
                    *state_toggle = !*state_toggle;
            }
        }

            [[maybe_unused]] [[nodiscard]] bool get_default_state() const {
            return default_state;
        }

        virtual MenuItem::Type get_type() const override {
            return MenuItem::Type::Toggle;
        }
    private:
        std::function<void(ToggleMenuItem &, bool, void *us_Data)> cb;
        bool callback_mode,
             *state_toggle,
             default_state{};
        void *us_data{};
    };

    struct [[maybe_unused]] CallbackMenuItem : public TextMenuItem {
        using CallbackType = std::function<void(const CallbackMenuItem &, void *)>;

        CallbackMenuItem(const char *label, CallbackType cb, void *us_data = nullptr) :
        TextMenuItem(label),
        cb(std::move(cb)),
        us_data(us_data) {}

        void operator()() {
            cb(*this, us_data);
        }

        virtual MenuItem::Type get_type() const override {
            return MenuItem::Type::Callback;
        }
    private:
        std::function<void(CallbackMenuItem &, void *us_Data)> cb;
        void *us_data;
    };

    struct [[maybe_unused]] Menu {
        Menu(const Menu &) = delete;
        Menu &operator= (Menu) = delete;

        Menu() = default;
        Menu(Menu&& menu) = default;

        template<
                typename M = MenuItem,
                typename ...A
                >
        typename std::enable_if_t<std::is_base_of_v<MenuItem, M>, void>
        addItem(A&& ...args) {
            items.emplace_back(std::make_unique<M>(args...));
        };

        void addSeparator() {
            this->addItem<SeparatorMenuItem>();
        }

        std::vector<std::unique_ptr<MenuItem>> items;
    };

    struct [[maybe_unused]] SubMenuItem : public TextMenuItem {
        std::unique_ptr<Menu> menu;

        explicit SubMenuItem(std::string label) :
                TextMenuItem(std::move(label)),
                menu(std::make_unique<Menu>()) {}

        explicit SubMenuItem(const char *label) :
                TextMenuItem(label),
                menu(std::make_unique<Menu>()) {}

        explicit SubMenuItem(std::string label, Menu &menu) :
                TextMenuItem(std::move(label)),
                menu(std::make_unique<Menu>(std::move(menu))) {}

        explicit SubMenuItem(const char *label, Menu &menu) :
                TextMenuItem(label),
                menu(std::make_unique<Menu>(std::move(menu))) {}

        ~SubMenuItem() override = default;


        virtual MenuItem::Type get_type() const override {
            return MenuItem::Type::SubMenu;
        }
    };
} // namespace Silver;