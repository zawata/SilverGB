#pragma once

#import <Cocoa/Cocoa.h>
#include <string>

/**
 * String Conversion
 **/
static NSString   *s_to_ns(const std::string &s) { return [NSString stringWithUTF8String:s.c_str()]; }

static std::string ns_to_s(NSString *s) { return std::string([s UTF8String]); }