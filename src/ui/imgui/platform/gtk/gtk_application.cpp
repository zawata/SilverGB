#include <cassert>
#include <gdkmm/event.h>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#include <epoxy/gl.h>
#include <gdkmm/enums.h>
#include <giomm/asyncresult.h>
#include <giomm/liststore.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/eventcontrollerlegacy.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/glarea.h>
#include <gtkmm/messagedialog.h>
#include <sigc++/adaptors/retype_return.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/sigc++.h>

#include "gb_core/defs.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui_impl_gtkmm.hpp"

#include "gtk_application.hpp"
#include "menu.hpp"

static const std::string class_name = "org.zawata.silver";

#define check_gl_error() _check_gl_error(__LINE__)
void _check_gl_error(u32 line) {
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        nowide::cerr << "GL" << line << ": " << as_hex(err) << std::endl;
    }
}

GtkApp::GtkApp(int argc, const char *argv[]) :
  Gtk::Application(class_name),
  app(new Silver::Application()) {
  app->onInit(argc, argv);
  app->window_cb.openFileDialog = sigc::mem_fun(*this, &GtkApp::openFileDialog);
  app->window_cb.openMessageBox = sigc::mem_fun(*this, &GtkApp::openMessageBox);
  keyboardBindings = new Silver::Binding::Tracker<guint>();

  // set default bindings
  keyboardBindings->mapButton(GDK_KEY_Z, Silver::Binding::Button::A);
  keyboardBindings->mapButton(GDK_KEY_X, Silver::Binding::Button::B);
  keyboardBindings->mapButton(GDK_KEY_Return, Silver::Binding::Button::Start);
  keyboardBindings->mapButton(GDK_KEY_BackSpace, Silver::Binding::Button::Select);
  keyboardBindings->mapButton(GDK_KEY_Up, Silver::Binding::Button::Up);
  keyboardBindings->mapButton(GDK_KEY_Down, Silver::Binding::Button::Down);
  keyboardBindings->mapButton(GDK_KEY_Left, Silver::Binding::Button::Left);
  keyboardBindings->mapButton(GDK_KEY_Right, Silver::Binding::Button::Right);
  // keyboardBindings->printKeyMap();
}

// filter format: "Label:ext,ext;Label2:ext;ext"
void GtkApp::openFileDialog(
    const std::string &title,
    const std::string &filters,
    std::function<void (const std::string&)> cb
) {
  auto file_dialog = Gtk::FileDialog::create();
  file_dialog->set_title(title);

  auto filterList = Gio::ListStore<Gtk::FileFilter>::create();
  std::stringstream ss(filters);
  std::string filter;

  while (std::getline(ss, filter, ';')) {
    std::cout << "start filter" << std::endl;
    auto fileFilter = Gtk::FileFilter::create();
    std::stringstream filter_ss(filter);

    if (filter.find(':') != std::string::npos) {
      std::string name;
      std::getline(filter_ss, name, ':');
      std::cout << "name: " << name << std::endl;
      fileFilter->set_name(name);
    }

    std::string next_pattern;
    while(std::getline(filter_ss, next_pattern, ',')) {
      std::cout << "parsed pattern " << next_pattern << std::endl;
      if (next_pattern.find('.') != std::string::npos) {
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

  this->window = new Gtk::Window();
  // this->window->set_resizable(false);
  this->window->set_size_request(GB_S_W, GB_S_H);

  this->gl_area = new Gtk::GLArea();
  this->gl_area->set_auto_render();

  // Connect gl area signals
  this->gl_area->signal_realize().connect(sigc::mem_fun(*this, &GtkApp::realize));
  this->gl_area->signal_unrealize().connect(sigc::mem_fun(*this, &GtkApp::unrealize), false);
  this->gl_area->signal_render().connect(sigc::mem_fun(*this, &GtkApp::render), false);

  this->window->set_child(*this->gl_area);
  add_window(*window);

  // we can't get generic "button press" events through gesture event controllers
  // and filling out 4-5 other event controllers for all the other event types
  // is so much boilerplate.
  auto legacyEventController = Gtk::EventControllerLegacy::create();
  legacyEventController->signal_event().connect(
      [this](const std::shared_ptr<const Gdk::Event> &event) -> bool {
        bool forward_event = true;
        switch (event->get_event_type()) {
          case Gdk::Event::Type::KEY_PRESS:
          case Gdk::Event::Type::KEY_RELEASE: {
            // convert keyvals to uppercase
            auto key = gdk_keyval_to_upper(event->get_keyval());
            if (this->keyboardBindings->isKeyMapped(key)) {
              this->keyboardBindings->setStateForKey(key, event->get_event_type() == Gdk::Event::Type::KEY_PRESS);
              Joypad::button_states_t button_states;
              this->keyboardBindings->getButtonStates(button_states);
              this->app->onUpdateInputs(button_states);
              forward_event = !this->app->app_state.game_capture_input;
            }

            break;
          }

          default:
            break;
        }

        if (forward_event) {
          return true;
        }

        return ImGui_ImplGtkmm_ProcessEvent(event.get());
      },
      false
  );
  window->add_controller(legacyEventController);

  // we need to force continous re-renders of the glArea
  window->add_tick_callback(
      [this](const Glib::RefPtr<Gdk::FrameClock>&) -> bool {
          this->gl_area->queue_render();
          return true;
      }
  );

  window->present();
}

void GtkApp::on_startup() {
  Gtk::Application::on_startup();

  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  this->create_menubar();
}

void GtkApp::on_shutdown() {
  Gtk::Application::on_shutdown();
  ImGui::DestroyContext();
}

void GtkApp::create_screen_texture() {
    //leave unpopulated right now so we can do partial updates later
    glGenTextures(1, &this->screen_texture);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, this->screen_texture);
    check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    check_gl_error();
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, GB_S_W, GB_S_H);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, 0);
    check_gl_error();

    this->app->screen_texture_id = (ImTextureID)screen_texture;
}

void GtkApp::create_menubar() {
  // call back to get menu
  auto menubarTemplate = new Silver::Menu();
  this->app->makeMenuBar(menubarTemplate);

  //deque for a breadth-first tree traversal
  std::deque<std::pair<Glib::RefPtr<Gio::Menu>, Silver::MenuItem *>> d;

  auto menubar = Gio::Menu::create();
  for(auto &i : menubarTemplate->items) d.emplace_back(menubar, i.get());

  while(!d.empty()) {
    Glib::RefPtr<Gio::Menu> gtk_menu = d.front().first;
    Silver::MenuItem *nuiMenuItem = d.front().second;
    d.pop_front();

    std::string partialActionName = std::to_string(nuiMenuItem->get_id());

    switch (nuiMenuItem->get_type()) {
      case Silver::MenuItem::SubMenu: {
        auto itemTemplate = dynamic_cast<Silver::SubMenuItem *>(nuiMenuItem);
        auto sub_menu = Gio::Menu::create();
        gtk_menu->append_submenu(itemTemplate->label, sub_menu);
        for(auto &i : itemTemplate->menu->items) {
          d.emplace_back(sub_menu, i.get());
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
        auto action = Gio::SimpleAction::create_bool(partialActionName);
        action->signal_change_state().connect([nuiMenuItem, &action](const Glib::VariantBase &b) -> void {
          auto itemTemplate = dynamic_cast<Silver::ToggleMenuItem *>(nuiMenuItem);
          action->set_state(b);

          //Damn you variant types
          bool new_state = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(b).get();

          (*itemTemplate)(new_state);
        });

        gtk_menu->append(itemTemplate->label, "app." + partialActionName);
        this->add_action(action);
        break;
      }

      case Silver::MenuItem::Callback: {
        auto itemTemplate = dynamic_cast<Silver::CallbackMenuItem *>(nuiMenuItem);
        auto action = Gio::SimpleAction::create(partialActionName);
        action->signal_activate().connect(sigc::hide([nuiMenuItem]() -> void {
          auto itemTemplate = dynamic_cast<Silver::CallbackMenuItem *>(nuiMenuItem);
          (*itemTemplate)();
        }));

        gtk_menu->append(itemTemplate->label, "app." + partialActionName);
        this->add_action(action);
        break;
      }

      // case Silver::MenuItem::Separator:
      // case Silver::MenuItem::Radio:
      case Silver::MenuItem::None:
      default: {
        std::cerr << "unrecognized menu item type" << std::endl;
      }
    }
  }

  Gtk::Application::set_menubar(menubar);
}

void GtkApp::realize() {
  this->gl_area->make_current();
  try {
    this->gl_area->throw_if_error();

    ImGui_ImplGtkmm_Init(this->window, this->gl_area);
    ImGui_ImplOpenGL3_Init();

    create_screen_texture();
  } catch(const Gdk::GLError& gle) {
    std::cerr << "An error occurred making the context current during realize:" << std::endl;
    std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
  }
}

void GtkApp::unrealize() {
  this->gl_area->make_current();
  try {
    this->gl_area->throw_if_error();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGtkmm_Shutdown();
  } catch(const Gdk::GLError& gle) {
    std::cerr << "An error occurred making the context current during unrealize" << std::endl;
    std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
  }
}

bool GtkApp::render(const Glib::RefPtr<Gdk::GLContext>& /* context */) {
  try {
    this->gl_area->throw_if_error();

    if(this->app->core) {
      // draw screen to texture
      glBindTexture(GL_TEXTURE_2D, screen_texture);
      glTexSubImage2D(
          GL_TEXTURE_2D,
          0,
          0,
          0,
          GB_S_W,
          GB_S_H,
          GL_RGB,
          GL_UNSIGNED_BYTE,
          this->app->core->getScreenBuffer()
      );
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    ImGui_ImplGtkmm_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::NewFrame();
    this->app->onDraw();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return true;
  } catch(const Gdk::GLError& gle) {
    std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    return false;
  }
}