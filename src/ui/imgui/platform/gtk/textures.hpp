#pragma once

#include "util/types/primitives.hpp"

#include "gtk_application.hpp"

struct GenericTexture {
    virtual ~GenericTexture() = default;

    using SizeType                          = std::pair<u16, u16>;

    virtual void    *getTextureId()         = 0;
    virtual SizeType getSize()              = 0;

    virtual void     update(GtkApp *gtkApp) = 0;
};

void buildTextures(GtkApp *gtkApp);
void updateTextures(GtkApp *gtkApp);