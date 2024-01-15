#include <AppKit/AppKit.h>
#include <iostream>
#include <deque>

#include <crt_externs.h>
#include <memory>

#include "app.hpp"
#import "cocoa/callback_list.hh"
#import "cocoa/app_delegate.hh"
#import "cocoa/app_view_controller.hh"
#import "cocoa/apple_menu.hh"

#include "menu.hpp"

#include "util.hh"

Silver::Application *g_app;

@implementation AppDelegate

- (instancetype)init:(NSString *)title {
    // self->windowDrawCallback = drawCallback;
    // self->windowDrawCallbackData = callbackData;
    if (self = [super init]) {
        g_app = new Silver::Application();
        g_app->window_cb.openFileDialog = [self](
                const std::string &title,
                const std::string &filters,
                const std::function<void(std::string)> &cb
        ) {
            [self openFileDialog:s_to_ns(title) filters:s_to_ns(filters) cb:cb];
        };
        g_app->window_cb.openMessageBox = [self](const std::string &title, const std::string &message) {
            [self openMessageBox:s_to_ns(title) message:s_to_ns(message)];
        };

        // bounds = NSMakeRect(0, 0, Silver::Window::DefaultWidth, Silver::Window::DefaultHeight);
        AppViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self->window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                backing:NSBackingStoreBuffered
                defer:NO];
        self->window.contentViewController = rootViewController;
        [self->window center];
        [self->window makeKeyAndOrderFront:self];

        int *argc = _NSGetArgc();
        char ***argv = _NSGetArgv();
        g_app->onInit(*argc, const_cast<const char **>(*argv));
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

    auto menubarTemplate = new Silver::Menu();
    g_app->makeMenuBar(menubarTemplate);

    //deque for a breadth-first tree traversal
    std::deque<std::pair<NSMenu *, Silver::MenuItem *>> d;

    //addx menus to processing list
    for(auto &i : menubarTemplate->items) {
        d.push_back({mainMenu, i.get()});
    }

    while(!d.empty()) {
        //the current submenu item
        NSMenu *mac_menu = d.front().first;
        //the NUIMenuItem Instance
        Silver::MenuItem *baseMenuItem = d.front().second;
        d.pop_front();

        switch(baseMenuItem->get_type()) {
            case Silver::MenuItem::SubMenu: {
                auto itemTemplate = dynamic_cast<Silver::SubMenuItem *>(baseMenuItem);
                std::string label = ((Silver::SubMenuItem *)baseMenuItem)->label;
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
                break;
            }

            case Silver::MenuItem::Toggle: {
                auto itemTemplate = dynamic_cast<Silver::ToggleMenuItem *>(baseMenuItem);
                NSMenuItem *sub_menu_item = [[NSMenuItem alloc]
                        initWithTitle:s_to_ns(itemTemplate->label)
                        action:@selector(onMenuCallback:)
                        keyEquivalent:@""];
                [sub_menu_item setTarget:[[MenuActionItem alloc] init:itemTemplate->get_id()]];

                [mac_menu addItem:sub_menu_item];

                add_menu_callback(itemTemplate->get_id(), std::make_pair(baseMenuItem, [=](Silver::MenuItem *baseMenuItem) {
                    bool current_state = sub_menu_item.state == NSControlStateValueOn;
                    (*(Silver::ToggleMenuItem *)baseMenuItem)(!current_state);
                    sub_menu_item.state = (!current_state) ? NSControlStateValueOn : NSControlStateValueOff;
                }));
                break;
            }

            case Silver::MenuItem::Callback: {
                auto itemTemplate = dynamic_cast<Silver::CallbackMenuItem *>(baseMenuItem);
                NSMenuItem *sub_menu_item = [[NSMenuItem alloc]
                        initWithTitle:s_to_ns(itemTemplate->label)
                            action:@selector(onMenuCallback:)
                        keyEquivalent:@""];
                [sub_menu_item setTarget:[[MenuActionItem alloc] init:itemTemplate->get_id()]];

                [mac_menu addItem:sub_menu_item];

                add_menu_callback(itemTemplate->get_id(), std::make_pair(baseMenuItem, [](Silver::MenuItem *baseMenuItem){
                    (*(Silver::CallbackMenuItem *)baseMenuItem)();
                }));
                break;
            }

            case Silver::MenuItem::Separator: {
                [mac_menu addItem:[NSMenuItem separatorItem]];
                break;
            }

            case Silver::MenuItem::None:
            case Silver::MenuItem::Text:
            case Silver::MenuItem::Radio:
            default:
                break;
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

- (void) openFileDialog:(NSString *)title
        filters:(NSString *)filters
        cb:(const std::function<void(std::string)> &)cb  {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];

    openPanel.title = title;
    openPanel.showsResizeIndicator = YES;
    openPanel.showsHiddenFiles = NO;
    openPanel.canChooseDirectories = NO;
    openPanel.canCreateDirectories = YES;
    openPanel.allowsMultipleSelection = NO;
    openPanel.allowedFileTypes = @[@"gb"];

    std::string path;
    if ([openPanel runModal] == NSModalResponseOK) {
        //get the selected file URLs
        NSURL *selection = openPanel.URLs[0];
        path = ns_to_s([[selection path] stringByResolvingSymlinksInPath]);
    }

    cb(path);
}

- (void) openMessageBox:(NSString *)title message:(NSString *)message  {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = title;
    alert.informativeText = message;
    [alert addButtonWithTitle: @"Save"];
    [alert addButtonWithTitle: @"Cancel"];
    [alert addButtonWithTitle: @"Don't Save"];

    if([alert runModal] == NSModalResponseOK) {
        // TODO
    }
}

@end