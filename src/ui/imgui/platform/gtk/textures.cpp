#include "textures.hpp"

struct ScreenTexture : public GenericTexture {
    ScreenTexture() {
        glGenTextures(1, &this->screen_texture);
        glBindTexture(GL_TEXTURE_2D, this->screen_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, Silver::Core::native_width, Silver::Core::native_height);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void *getTextureId() override {
        // dirty but necessary for imgui
        return reinterpret_cast<void *>(screen_texture);
    }

    SizeType getSize() override {
        return {
            Silver::Core::native_width,
            Silver::Core::native_height
        };
    }

    void update(GtkApp *gtkApp) override {
        // core isn't guaranteed to exist here
        if(!gtkApp->app->core) {
            return;
        }

        // this type and format must match the texture pixel format
        Silver::PixelBufferEncoder<u8>::encodePixelBuffer<Silver::PixelFormat::RGB>(
                screen_buffer,
                Silver::Core::native_pixel_count * 4,
                gtkApp->app->core->getPixelBuffer()
        );

        // draw screen to texture
        glBindTexture(GL_TEXTURE_2D, this->screen_texture);
        glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0,
                0,
                Silver::Core::native_width,
                Silver::Core::native_height,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                screen_buffer
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    GLuint screen_texture = 0;
    u8 screen_buffer[Silver::Core::native_pixel_count * 4] = {0};
};

struct BackgroundDebugTexture : public GenericTexture {
    BackgroundDebugTexture() {
        glGenTextures(1, &this->bg_debug_texture);
        GLuint debug_bg_texture;
        glGenTextures(1, &debug_bg_texture);
        glBindTexture(GL_TEXTURE_2D, debug_bg_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 256, 256);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void *getTextureId() override {
        // dirty but necessary for imgui
        return reinterpret_cast<void *>(bg_debug_texture);
    }

    SizeType getSize() override {
        return { 256, 256 };
    }

    void update(GtkApp *gtkApp) override {
        // core isn't guaranteed to exist here
        if(!gtkApp->app->core) {
            return;
        }

        gtkApp->app->core->getBGBuffer(pixelBuf);
        Silver::PixelBufferEncoder<u8>::encodePixelBuffer<Silver::PixelFormat::RGB>(
                debug_buffer,
                Silver::Core::native_pixel_count * 3,
                pixelBuf
        );

        // draw screen to texture
        glBindTexture(GL_TEXTURE_2D, this->bg_debug_texture);
        glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0,
                0,
                256,
                256,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                debug_buffer
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    GLuint bg_debug_texture = 0;
    u8 debug_buffer[256 * 256 * 3] = {0};
    std::vector<Silver::Pixel> pixelBuf;
};

struct VRAMTileDebugTexture : public GenericTexture {
    VRAMTileDebugTexture(int sectionIdx, bool bank1): sectionIdx(sectionIdx), bank1(bank1) {
        glGenTextures(1, &this->vram_debug_texture);
        GLuint debug_bg_texture;
        glGenTextures(1, &debug_bg_texture);
        glBindTexture(GL_TEXTURE_2D, debug_bg_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 256, 256);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void *getTextureId() override {
        // dirty but necessary for imgui
        return reinterpret_cast<void *>(vram_debug_texture);
    }

    SizeType getSize() override {
        return { 128, 64 };
    }

    void update(GtkApp *gtkApp) override {
        // core isn't guaranteed to exist here
        if(!gtkApp->app->core) {
            return;
        }

        gtkApp->app->core->getVRAMBuffer(pixelBuf, sectionIdx, bank1);
        Silver::PixelBufferEncoder<u8>::encodePixelBuffer<Silver::PixelFormat::RGB>(
                debug_buffer,
                128 * 64 * 3,
                pixelBuf
        );

        // draw screen to texture
        glBindTexture(GL_TEXTURE_2D, this->vram_debug_texture);
        glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0,
                0,
                128,
                64,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                debug_buffer
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    u8 sectionIdx;
    bool bank1;

    GLuint vram_debug_texture = 0;
    u8 debug_buffer[256 * 256 * 3] = {0};
    std::vector<Silver::Pixel> pixelBuf;
};

void buildTextures(GtkApp *gtkApp) {
    gtkApp->screenTex = new ScreenTexture();
    gtkApp->app->screen_texture_id = gtkApp->screenTex->getTextureId();
    gtkApp->backgroundDebugTex = new BackgroundDebugTexture();
    gtkApp->app->debug_bg_texture_id = gtkApp->backgroundDebugTex->getTextureId();

    for(int i = 0; i < 3; i++) {
        int idx = i << 1;
        gtkApp->vramDebugTex[idx] = new VRAMTileDebugTexture(i, false);
        gtkApp->app->vram_debug_texture_ids[idx] = gtkApp->vramDebugTex[idx]->getTextureId();
        gtkApp->vramDebugTex[idx+1] = new VRAMTileDebugTexture(i, true);
        gtkApp->app->vram_debug_texture_ids[idx+1] = gtkApp->vramDebugTex[idx+1]->getTextureId();
    }
}

void updateTextures(GtkApp *gtkApp) {
    gtkApp->screenTex->update(gtkApp);
    gtkApp->backgroundDebugTex->update(gtkApp);

    for(int i = 0; i < 3; i++) {
        int idx = i << 1;
        gtkApp->vramDebugTex[idx]->update(gtkApp);
        gtkApp->vramDebugTex[idx+1]->update(gtkApp);
    }
}