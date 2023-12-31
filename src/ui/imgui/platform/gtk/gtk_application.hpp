#pragma once

#include <gtkmm/application.h>

#include <gtkmm/glarea.h>
#include <iostream>
#include <cassert>

#include "menu.hpp"
#include "binding.hpp"
#include "app.hpp"

class GtkApp : public Gtk::Application {
public:
  GtkApp(int argc, const char *argv[]);

protected:
  // Override default signal handlers:
  void on_activate() override;
  void on_startup() override;
  void on_shutdown() override;

  void openFileDialog(
      const std::string &title,
      const std::string &filters,
      std::function<void (const std::string&)> cb
  );
  void openMessageBox(const std::string &title, const std::string &message);

  void realize();
  void unrealize();
  bool render(const Glib::RefPtr<Gdk::GLContext>& /* context */);

  void create_menubar();
  void create_screen_texture();

private:
  Gtk::Window *window;
  Gtk::GLArea *gl_area;
  u32 screen_texture;
  Silver::Application *app;
  Silver::Binding::Tracker<guint> *keyboardBindings; // GDK_KEY_*
};