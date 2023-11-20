#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "cocoa/app_delegate.hh"

int main(int argc, const char *argv[]) {
    [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
    [[NSApplication sharedApplication] setDelegate:[[AppDelegate alloc] init:@"SilverGB"]];
    return NSApplicationMain(argc, argv);
}