#include "../platform.hpp"
#include "gtk_application.hpp"

// FIXME: doesn't seem safe
extern GtkApp *global_gtkApp;

void           Silver::Platform::createMenuBar() { global_gtkApp->createMenuBar(); }

void           Silver::Platform::openFileDialog(
        const std::string &title, const std::string &filters, std::function<void(const std::string &)> cb) {
    global_gtkApp->openFileDialog(title, filters, cb);
}

void Silver::Platform::openMessageBox(const std::string &title, const std::string &message) {
    global_gtkApp->openMessageBox(title, message);
}
