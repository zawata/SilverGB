#include <iostream>

#include "gtk_application.hpp"

GtkApp *global_gtkApp;

int     main(int argc, const char *argv[]) {
    global_gtkApp = new GtkApp(argc, argv);
    return global_gtkApp->run();
}
