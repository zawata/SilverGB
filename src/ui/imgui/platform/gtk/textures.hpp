#pragma once

#include <epoxy/gl.h>

#include "gtk_application.hpp"
#include "gb_core/core.hpp"
#include "util/types/primitives.hpp"

struct GenericTexture {
    using SizeType = std::pair<u16, u16>;

    virtual void *getTextureId() = 0;
    virtual SizeType getSize() = 0;

    virtual void update(GtkApp *gtkApp) = 0;
};

void buildTextures(GtkApp *gtkApp);
void updateTextures(GtkApp *gtkApp);