#include <iostream>

#include "gtk_application.hpp"

int main (int argc, const char *argv[]) {
    auto application = new GtkApp(argc, argv);
    return application->run();
}