#pragma once

#include <functional>
#include <string>

namespace Silver::Platform {
    void createMenuBar();
    void
    openFileDialog(const std::string &title, const std::string &filters, std::function<void(const std::string &)> cb);
    void openMessageBox(const std::string &title, const std::string &message);
};