#pragma once

#import <Cocoa/Cocoa.h>
#include <list>

#include "app.hpp"

@interface AppDelegate: NSObject <NSApplicationDelegate> {
    NSWindow                                  *window;
    NSView                                    *view;
    // NSRect bounds;
    std::list<std::pair<NSString *, NSMenu *>> menu_list;
}

- (instancetype)init:(NSString *)title;

- (void)openMessageBox:(NSString *)title message:(NSString *)message;
- (NSString *)openFileDialog:(NSString *)title filters:(NSString *)filters;

@end