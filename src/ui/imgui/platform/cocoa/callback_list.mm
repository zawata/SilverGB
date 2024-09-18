#import "cocoa/callback_list.hh"
#include "util/log.hpp"

#include <unordered_map>

static std::unordered_map<int, std::pair<Silver::MenuItem *, std::function<void (Silver::MenuItem *)>>> callback_map;

void add_menu_callback(int menu_id, std::pair<Silver::MenuItem *, std::function<void (Silver::MenuItem *)>> p) {
    callback_map.insert({menu_id, p});
}

@implementation MenuActionItem

-(id)init:(int)item_id {
    self->menu_id = item_id;

    return self;
}

-(void)onMenuCallback:(id)sender {
    try {
        auto menu_comp = callback_map.at(self->menu_id);
        menu_comp.second(menu_comp.first);
    } catch(std::out_of_range &) {
        LogError("MenuActionItem") << "onMenuCallback: menu ID not found";
    }
}
@end