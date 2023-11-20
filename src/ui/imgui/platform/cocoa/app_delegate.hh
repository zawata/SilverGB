#pragma once

#include <list>

#import <Cocoa/Cocoa.h>

#include "common/window.hpp"
#include "app.hpp"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
	NSWindow *window;
	NSView* view;
    Silver::Application *app;
	// NSRect bounds;
	// std::list<std::pair<NSString *, NSMenu *>> menu_list;
}

- (instancetype) init:(NSString *)title;

@end