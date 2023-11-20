#import "apple_menu.hh"

// courtesy of SFML
// https://github.com/SFML/SFML/blob/72d88033e2f24be0fb1e9df614a56f3d5274154c/src/SFML/Window/OSX/SFApplication.m#L89

NSMenu *createAppleMenu() {
    // Apple menu is as follows:
    //
    // AppName >
    //    About AppName
    //    --------------------
    //    Preferences...    [greyed]
    //    --------------------
    //    Services >
    //        / default empty menu /
    //    --------------------
    //    Hide AppName      Command+H
    //    Hide Others       Option+Command+H
    //    Show All
    //    --------------------
    //    Quit AppName      Command+Q

    NSString* appName = [NSRunningApplication currentApplication].localizedName;

    // APPLE MENU
    NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];

    // ABOUT
    [appleMenu
        addItemWithTitle:[@"About " stringByAppendingString:appName]
        action:@selector( :)
        keyEquivalent:@""];

    // SEPARATOR
    [appleMenu addItem:[NSMenuItem separatorItem]];

    // PREFERENCES
    [appleMenu
        addItemWithTitle:@"Preferences..."
        action:nil
        keyEquivalent:@""];

    // SEPARATOR
    [appleMenu addItem:[NSMenuItem separatorItem]];

    // SERVICES
    NSMenu* serviceMenu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
    NSMenuItem* serviceItem = [appleMenu
        addItemWithTitle:@"Services"
        action:nil
        keyEquivalent:@""];

    [serviceItem setSubmenu:serviceMenu];
    [[NSApplication sharedApplication] setServicesMenu:serviceMenu];

    // SEPARATOR
    [appleMenu addItem:[NSMenuItem separatorItem]];

    // HIDE
    [appleMenu addItemWithTitle:
        [@"Hide " stringByAppendingString:appName]
                         action:@selector(hide:)
                  keyEquivalent:@"h"];

    // HIDE OTHER
    NSMenuItem* hideOtherItem = [appleMenu
        addItemWithTitle:@"Hide Others"
        action:@selector(hideOtherApplications:)
        keyEquivalent:@"h"];
    [hideOtherItem setKeyEquivalentModifierMask:(NSAlternateKeyMask | NSCommandKeyMask)];

    // SHOW ALL
    [appleMenu
        addItemWithTitle:@"Show All"
        action:@selector(unhideAllApplications:)
        keyEquivalent:@""];

    // SEPARATOR
    [appleMenu addItem:[NSMenuItem separatorItem]];

    // QUIT
    [appleMenu addItemWithTitle:[@"Quit " stringByAppendingString:appName]
        action:@selector(terminate:)
        keyEquivalent:@"q"];

    return appleMenu;
}

NSMenu *createWindowMenu() {
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];

    // MINIMIZE
    NSMenuItem* minimizeItem = [[NSMenuItem alloc]
        initWithTitle:@"Minimize"
        action:@selector(performMiniaturize:)
        keyEquivalent:@"m"];
    [windowMenu addItem:minimizeItem];
    [minimizeItem release];

    // ZOOM
    [windowMenu
        addItemWithTitle:@"Zoom"
        action:@selector(performZoom:)
        keyEquivalent:@""];

    // SEPARATOR
    [windowMenu addItem:[NSMenuItem separatorItem]];

    // BRING ALL TO FRONT
    [windowMenu
        addItemWithTitle:@"Bring All to Front"
        action:@selector(bringAllToFront:)
        keyEquivalent:@""];

    return windowMenu;
}