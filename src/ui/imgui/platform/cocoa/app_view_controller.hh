#pragma once

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include <memory>

namespace Silver {
    struct Application;
}

@interface AppViewController : NSViewController<NSWindowDelegate> {
}
@end

@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLTexture> screen_texture;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@end