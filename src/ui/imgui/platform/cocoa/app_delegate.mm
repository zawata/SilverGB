#include <iostream>
#include <deque>

#include <crt_externs.h>

#import "cocoa/callback_list.hh"
#import "cocoa/app_delegate.hh"
#import "cocoa/app_view_controller.hh"
#import "cocoa/apple_menu.hh"

#include "common/menu.hpp"

#include "util.hh"

@implementation AppDelegate

- (instancetype)init:(NSString *)title {
    // self->windowDrawCallback = drawCallback;
    // self->windowDrawCallbackData = callbackData;
    if (self = [super init]) {
        // bounds = NSMakeRect(0, 0, NUI::Window::DefaultWidth, NUI::Window::DefaultHeight);
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self->window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self->window.contentViewController = rootViewController;
        [self->window center];
        [self->window makeKeyAndOrderFront:self];

        self->app = new Silver::Application();
        self->app->onInit(_NSGetArgc(), _NSGetArgv());
    }
    return self;

}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    std::cout << "applicationWillFinishLaunching" << std::endl;

    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    std::cout << "applicationDidFinishLaunching" << std::endl;

    NSMenu *mainMenu = [NSApp mainMenu];
    mainMenu = [[[NSMenu alloc] initWithTitle:@"MainMenu"] autorelease];
    [NSApp setMainMenu:mainMenu];

    NSMenuItem *appleItem = [[NSApp mainMenu] addItemWithTitle:@"" action:nil keyEquivalent:@""];
    NSMenu *appleMenu = createAppleMenu();
    [appleItem setSubmenu:appleMenu];

    auto menubarTemplate = new NUI::Menu();
    self->app->makeMenuBar(menubarTemplate);

    //deque for a breadth-first tree traversal
    std::deque<std::pair<NSMenu *, NUI::MenuItem *>> d;

    //addx menus to processing list
    for(auto &i : menubarTemplate->items) {
        d.push_back({mainMenu, i.get()});
    }

    while(!d.empty()) {
        //the current submenu item
        NSMenu *mac_menu = d.front().first;
        //the NUIMenuItem Instance
        NUI::MenuItem *nui_m = d.front().second;
        d.pop_front();

        if(nui_m->is_SubMenuItem()) {
            auto itemTemplate = dynamic_cast<NUI::SubMenuItem *>(nui_m);
            std::string label = ((NUI::SubMenuItem *)nui_m)->label;
            NSMenuItem *sub_menu_item = [[NSMenuItem alloc]
                    initWithTitle:s_to_ns(label)
                           action:nil
                    keyEquivalent:@""];
            NSMenu *sub_menu = [[NSMenu alloc] initWithTitle:s_to_ns(label)];

            [sub_menu_item setSubmenu:sub_menu];

            if(mac_menu) {
                [mac_menu addItem:sub_menu_item];
            }

            for(auto &i : itemTemplate->menu->items) {
                d.push_back({sub_menu, i.get()});
            }
        } else if(nui_m->is_ToggleMenuItem()) {
            auto itemTemplate = dynamic_cast<NUI::ToggleMenuItem *>(nui_m);
            NSMenuItem *sub_menu_item = [[NSMenuItem alloc]
                    initWithTitle:s_to_ns(itemTemplate->label)
                           action:@selector(onMenuCallback:)
                    keyEquivalent:@""];
            [sub_menu_item setTarget:[[MenuActionItem alloc] init:itemTemplate->get_id()]];

            [mac_menu addItem:sub_menu_item];

            add_menu_callback(itemTemplate->get_id(), std::make_pair(nui_m, [=](NUI::MenuItem *nui_m) {
                bool current_state = sub_menu_item.state == NSControlStateValueOn;
                (*(NUI::ToggleMenuItem *)nui_m)(!current_state);
                sub_menu_item.state = (!current_state) ? NSControlStateValueOn : NSControlStateValueOff;
            }));
        } else if(nui_m->is_CallbackMenuItem()) {
            auto itemTemplate = dynamic_cast<NUI::CallbackMenuItem *>(nui_m);
            NSMenuItem *sub_menu_item = [[NSMenuItem alloc]
                    initWithTitle:s_to_ns(itemTemplate->label)
                           action:@selector(onMenuCallback:)
                    keyEquivalent:@""];
            [sub_menu_item setTarget:[[MenuActionItem alloc] init:itemTemplate->get_id()]];

            [mac_menu addItem:sub_menu_item];

            add_menu_callback(itemTemplate->get_id(), std::make_pair(nui_m, [](NUI::MenuItem *nui_m){
                (*(NUI::CallbackMenuItem *)nui_m)();
            }));
        } else if(nui_m->is_SeparatorMenuItem()) {
            [mac_menu addItem:[NSMenuItem separatorItem]];
        }
    }

    // build menu
    NSMenuItem* windowItem = [mainMenu
        addItemWithTitle:@""
        action:nil
        keyEquivalent:@""];
    NSMenu *windowMenu = createWindowMenu();
    [windowItem setSubmenu:windowMenu];
    [[NSApplication sharedApplication] setWindowsMenu:windowMenu];

    [window makeKeyAndOrderFront:self];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // TODO
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender {
    // TODO
    return nil;
}

- (void)didSelectMySection {
    std::cout << "didSelectMySection" << std::endl;
}

- (void)didSelectClickMe {
    std::cout << "didSelectClickMe" << std::endl;
}

@end

