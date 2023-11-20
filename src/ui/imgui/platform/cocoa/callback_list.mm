#import "cocoa/callback_list.hh"

#include <unordered_map>

static std::unordered_map<int, std::pair<NUI::MenuItem *, std::function<void (NUI::MenuItem *)>>> callback_map;

void add_menu_callback(int menu_id, std::pair<NUI::MenuItem *, std::function<void (NUI::MenuItem *)>> p) {
    callback_map.insert({menu_id, p});
}

@implementation MenuActionItem

-(id)init:(int)item_id {
    self->menu_id = item_id;

    return self;
}

-(void)onMenuCallback:(id)sender {
    std::cout << "onMenuCallback" << std::endl;
    try {
        auto menu_comp = callback_map.at(self->menu_id);
        menu_comp.second(menu_comp.first);
    } catch(std::out_of_range &) {
        std::cerr << "onMenuCallback: menu ID not found" << std::endl;
    }
}
@end