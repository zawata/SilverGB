#pragma once

#include <cassert>
#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/glarea.h>
#include <iostream>

#include "app.hpp"
#include "binding.hpp"
#include "menu.hpp"

struct GenericTexture;

class GtkApp: public Gtk::Application {
public:
    GtkApp(int argc, const char *argv[]);

    // Override default signal handlers:
    void on_activate() override;
    void on_startup() override;
    void on_shutdown() override;

    void
    openFileDialog(const std::string &title, const std::string &filters, std::function<void(const std::string &)> cb);

    void                    openMessageBox(const std::string &title, const std::string &message);

    bool                    handleEvent(const std::shared_ptr<const Gdk::Event> &event);

    void                    realize();
    void                    unrealize();
    bool                    render(const Glib::RefPtr<Gdk::GLContext>                    &/* context */);

    void                    createMenuBar();

    Gtk::ApplicationWindow *window;
    Gtk::GLArea            *gl_area;
    Silver::Application    *app;

    // textures
    GenericTexture         *screenTex;
    GenericTexture         *backgroundDebugTex;
    GenericTexture         *vramDebugTex[6];
};