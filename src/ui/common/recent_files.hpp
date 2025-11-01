#pragma once

#include <algorithm>
#include <list>
#include <string>

class RecentFiles {
    std::list<std::string> recent_files;

public:
    RecentFiles() :
        recent_files({}) { }
    RecentFiles(std::initializer_list<std::string> recent_files) :
        recent_files(recent_files) { }

    RecentFiles(const RecentFiles &)             = delete;
    RecentFiles &operator= (const RecentFiles &) = delete;
    RecentFiles(RecentFiles &&)                  = delete;
    RecentFiles &operator= (RecentFiles &&)      = delete;

    void         insert(std::string const &file) {
        recent_files.remove(file);
        recent_files.push_front(file);
        if(recent_files.size() > 10) {
            recent_files.pop_back();
        }
    }

    std::_List_const_iterator<std::string> begin() const { return recent_files.cbegin(); }
    std::_List_const_iterator<std::string> end() const { return recent_files.cend(); }
    void                                   clear() { recent_files.clear(); }
};