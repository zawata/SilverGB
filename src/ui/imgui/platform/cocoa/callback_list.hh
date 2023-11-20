#pragma once

#import <Cocoa/Cocoa.h>

#include "common/menu.hpp"

void add_menu_callback(int, std::pair<NUI::MenuItem *, std::function<void (NUI::MenuItem *)>>);

@interface MenuActionItem : NSObject {
    int menu_id;
}

-(id)init:(int)menu_id;
-(void)onMenuCallback:(id)sender;
@end