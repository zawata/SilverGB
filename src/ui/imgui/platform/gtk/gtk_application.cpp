#include "gtk_application.hpp"

#include <epoxy/gl.h>
#include <gdkmm/enums.h>
#include <gdkmm/event.h>
#include <giomm/asyncresult.h>
#include <giomm/liststore.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/eventcontrollerlegacy.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/glarea.h>
#include <gtkmm/messagedialog.h>
#include <iostream>
#include <memory>
#include <sigc++/adaptors/retype_return.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/sigc++.h>
#include <sstream>
#include <string>

#include "gb_core/core.hpp"

#include "util/log.hpp"

#include "binding.hpp"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui_impl_gtkmm.hpp"
#include "textures.hpp"

static const std::string class_name = "org.zawata.silver";

#define check_gl_error() _check_gl_error(__LINE__)

void _check_gl_error(u32 line) {
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        LogError("GtkApp") << "GL" << line << ": " << as_hex(err);
    }
}

GtkApp::GtkApp(int argc, const char *argv[]) :
    Gtk::Application(class_name), app(new Silver::Application()) {
    app->onInit(argc, argv);
    app->window_cb.openFileDialog = sigc::mem_fun(*this, &GtkApp::openFileDialog);
    app->window_cb.openMessageBox = sigc::mem_fun(*this, &GtkApp::openMessageBox);

    // keyboardBindings->printKeyMap();
}

// filter format: "Label:ext,ext;Label2:ext;ext"
void GtkApp::openFileDialog(
        const std::string &title, const std::string &filters, std::function<void(const std::string &)> cb) {
    auto file_dialog = Gtk::FileDialog::create();
    file_dialog->set_title(title);

    auto              filterList = Gio::ListStore<Gtk::FileFilter>::create();
    std::stringstream ss(filters);
    std::string       filter;

    while(std::getline(ss, filter, ';')) {
        auto              fileFilter = Gtk::FileFilter::create();
        std::stringstream filter_ss(filter);

        if(filter.find(':') != std::string::npos) {
            std::string name;
            std::getline(filter_ss, name, ':');
            fileFilter->set_name(name);
        }

        std::string next_pattern;
        while(std::getline(filter_ss, next_pattern, ',')) {
            if(next_pattern.find('.') != std::string::npos) {
                fileFilter->add_pattern(next_pattern);
            } else {
                fileFilter->add_pattern("*." + next_pattern);
            }
        }

        filterList->append(fileFilter);
    }

    file_dialog->set_filters(filterList);
    file_dialog->open([file_dialog, this, cb](std::shared_ptr<Gio::AsyncResult> &result) {
        auto file = file_dialog->open_finish(result);
        cb(file->get_path());
    });
}

// TODO: callback
void GtkApp::openMessageBox(const std::string &title, const std::string &message) {
    auto message_box = Gtk::MessageDialog(title);
    message_box.set_secondary_text(message);
    message_box.set_application(std::shared_ptr<Gtk::Application>(this));
    message_box.show();
}

void GtkApp::on_activate() {
    Gtk::Application::on_activate();

    this->window  = new Gtk::ApplicationWindow();

    this->gl_area = new Gtk::GLArea();
    this->gl_area->set_auto_render();
    this->gl_area->set_size_request(Silver::Core::native_width * 2, Silver::Core::native_height * 2);

    // Connect gl area signals
    this->gl_area->signal_realize().connect(sigc::mem_fun(*this, &GtkApp::realize));
    this->gl_area->signal_unrealize().connect(sigc::mem_fun(*this, &GtkApp::unrealize), false);
    this->gl_area->signal_render().connect(sigc::mem_fun(*this, &GtkApp::render), false);

    this->window->set_child(*this->gl_area);
    this->window->set_show_menubar();
    this->add_window(*window);

    // we can't get generic "button press" events through gesture event controllers
    // and filling out 4-5 other event controllers for all the other event types
    // is so much boilerplate.
    auto legacyEventController = Gtk::EventControllerLegacy::create();
    legacyEventController->signal_event().connect(
            [this](const std::shared_ptr<const Gdk::Event> &event) -> bool { return this->handleEvent(event); }, false);
    this->window->add_controller(legacyEventController);

    // we need to force continuous re-renders of the glArea
    window->add_tick_callback([this](const Glib::RefPtr<Gdk::FrameClock> &) -> bool {
        this->gl_area->queue_render();
        return true;
    });

    window->present();
}

bool GtkApp::handleEvent(const std::shared_ptr<const Gdk::Event> &event) {
    bool isKeyPressEvent    = event->get_event_type() == Gdk::Event::Type::KEY_PRESS;
    bool isKeyReleasedEvent = event->get_event_type() == Gdk::Event::Type::KEY_RELEASE;
    bool isKeyEvent         = isKeyPressEvent || isKeyReleasedEvent;

    if(isKeyEvent) {
        if(event->get_keyval() == GDK_KEY_Escape && this->app->binding->isRecording()) {
            this->app->binding->stopRecording();
            return true;
        }

        bool actionIsMapped = this->app->binding->sendInput(
                Silver::Binding::makeKeyboardInput(event->get_keycode()), isKeyPressEvent);

        if(actionIsMapped) {
            return true;
        }
    }

    return ImGui_ImplGtkmm_ProcessEvent(event.get());
}

void GtkApp::on_startup() {
    Gtk::Application::on_startup();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    this->createMenuBar();
}

void GtkApp::on_shutdown() {
    Gtk::Application::on_shutdown();
    ImGui::DestroyContext();
}

void GtkApp::createMenuBar() {
    LogDebug("GtkApp") << "Creating menu bar";

    static bool buildingMenuBar = false;
    if(buildingMenuBar) {
        assert(!buildingMenuBar);
    }

    buildingMenuBar      = true;

    // call back to get menu
    auto menubarTemplate = new Silver::Menu();
    this->app->makeMenuBar(menubarTemplate);

    // deque for a breadth-first tree traversal
    std::deque<std::pair<Glib::RefPtr<Gio::Menu>, Silver::MenuItem *>> d;

    auto                                                               menubar = Gio::Menu::create();
    for(auto &i : menubarTemplate->items) {
        d.emplace_back(menubar, i.get());
    }

    while(!d.empty()) {
        auto              gtk_menu    = d.front().first;
        Silver::MenuItem *nuiMenuItem = d.front().second;
        d.pop_front();

        std::string partialActionName = std::to_string(nuiMenuItem->get_id());

        switch(nuiMenuItem->get_type()) {
        case Silver::MenuItem::SubMenu: {
            auto itemTemplate = dynamic_cast<Silver::SubMenuItem *>(nuiMenuItem);
            auto sub_menu     = Gio::Menu::create();
            gtk_menu->append_submenu(itemTemplate->label, sub_menu);

            Glib::RefPtr<Gio::Menu> section_menu = Gio::Menu::create();
            sub_menu->append_section(section_menu);

            for(auto &item : itemTemplate->menu->items) {
                if(item->get_type() == Silver::MenuItem::Separator) {
                    section_menu = Gio::Menu::create();
                    sub_menu->append_section(section_menu);
                } else {
                    d.emplace_back(section_menu, item.get());
                }
            }
            break;
        }

        case Silver::MenuItem::Text: {
            auto itemTemplate = dynamic_cast<Silver::TextMenuItem *>(nuiMenuItem);
            // the action name is a formality here since a text-menu item won't
            // have a function associated with it
            gtk_menu->append(itemTemplate->label, "app." + partialActionName);
            break;
        }

        case Silver::MenuItem::Toggle: {
            auto itemTemplate = dynamic_cast<Silver::ToggleMenuItem *>(nuiMenuItem);
            auto action       = Gio::SimpleAction::create_bool(partialActionName);
            action->signal_change_state().connect([itemTemplate, action](const Glib::VariantBase &b) -> void {
                action->set_state(b);

                // Damn you variant types
                bool new_state = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(b).get();

                (*itemTemplate)(new_state);
            });

            gtk_menu->append(itemTemplate->label, "app." + partialActionName);
            this->add_action(action);
            break;
        }

        case Silver::MenuItem::Callback: {
            auto itemTemplate = dynamic_cast<Silver::CallbackMenuItem *>(nuiMenuItem);
            auto action       = Gio::SimpleAction::create(partialActionName);
            action->signal_activate().connect(sigc::hide([itemTemplate]() -> void { (*itemTemplate)(); }));

            gtk_menu->append(itemTemplate->label, "app." + partialActionName);
            this->add_action(action);
            break;
        }

        // case Silver::MenuItem::Separator:
        // case Silver::MenuItem::Radio:
        case Silver::MenuItem::None:
        default:                     {
            LogError("GtkApp") << "unrecognized menu item type";
        }
        }
    }

    Gtk::Application::set_menubar(menubar);
    buildingMenuBar = false;
}

void GtkApp::realize() {
    this->gl_area->make_current();
    try {
        this->gl_area->throw_if_error();

        ImGui_ImplGtkmm_Init(this->window, this->gl_area);
        ImGui_ImplOpenGL3_Init();

        buildTextures(this);
    } catch(const Gdk::GLError &gle) {
        LogError("GtkApp") << "An error occurred making the context current during realize:";
        LogError("GtkApp") << gle.domain() << "-" << gle.code() << "-" << gle.what();
    }
}

void GtkApp::unrealize() {
    this->gl_area->make_current();
    try {
        this->gl_area->throw_if_error();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGtkmm_Shutdown();
    } catch(const Gdk::GLError &gle) {
        LogError("GtkApp") << "An error occurred making the context current during unrealize";
        LogError("GtkApp") << gle.domain() << "-" << gle.code() << "-" << gle.what();
    }
}

u8   screen_buffer[Silver::Core::native_pixel_count * 4] = {0};
u8   debug_buffer[256 * 256 * 3]                         = {0};

// serves as our run-loop
bool GtkApp::render(const Glib::RefPtr<Gdk::GLContext> & /* context */) {
    try {
        this->gl_area->throw_if_error();

        updateTextures(this);

        ImGui_ImplGtkmm_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::NewFrame();
        this->app->onUpdate();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        return true;
    } catch(const Gdk::GLError &gle) {
        LogError("GtkApp") << gle.domain() << "-" << gle.code() << "-" << gle.what();
        return false;
    }
}
