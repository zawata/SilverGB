#pragma once

#import <Cocoa/Cocoa.h>

#include "menu.hpp"

void       add_menu_callback(int, std::pair<Silver::MenuItem *, std::function<void(Silver::MenuItem *)>>);

@interface MenuActionItem: NSObject {
    int menu_id;
}

- (id)init:(int)menu_id;
- (void)onMenuCallback:(id)sender;
@end