#include <chrono>
#include <iostream>

#include "gb_core/core.hpp"

#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include "SDL_events.h"
#include "SDL_keycode.h"

#include "audio.hpp"
#include "gb_core/defs.hpp"
#include "util/file.hpp"

GLuint screen_texture,
       shader_program;

#define check_gl_error() _check_gl_error(__LINE__)
void _check_gl_error(u32 line) {
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        nowide::cerr << "GL" << line << ": " << as_hex(err) << std::endl;
    }
}

void gl_init_textures() {
    //Build Screen Texture, leave unpopulated right now so we can do partial updates later
    glGenTextures(1, &screen_texture);                                check_gl_error();
    glBindTexture(GL_TEXTURE_2D, screen_texture);                     check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); check_gl_error();
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, GB_S_W, GB_S_H);        check_gl_error();
    glBindTexture(GL_TEXTURE_2D, 0);                                  check_gl_error();
}

void gl_init_shaders() {
    std::string vert_shader_source = "                         \
        #version 150 core                                    \n\
        out vec2 v_tex;                                      \n\
                                                             \n\
        const vec2 v_pos[4]=vec2[4](vec2(-1.0, 1.0),         \n\
                                    vec2(-1.0,-1.0),         \n\
                                    vec2( 1.0, 1.0),         \n\
                                    vec2( 1.0,-1.0));        \n\
        const vec2 t_pos[4]=vec2[4](vec2(0.0,0.0),           \n\
                                    vec2(0.0,1.0),           \n\
                                    vec2(1.0,0.0),           \n\
                                    vec2(1.0,1.0));          \n\
                                                             \n\
        void main()                                          \n\
        {                                                    \n\
            v_tex=t_pos[gl_VertexID];                        \n\
            gl_Position=vec4(v_pos[gl_VertexID], 0.0, 1.0);  \n\
        }";

    std::string frag_shader_source = "                         \
        #version 150 core                                    \n\
        in vec2 v_tex;                                       \n\
        uniform sampler2D texSampler;                        \n\
        out vec4 color;                                      \n\
        void main()                                          \n\
        {                                                    \n\
            color=texture(texSampler, v_tex);                \n\
        }";

    GLuint vert_shader, frag_shader;
    const char *c_str;
    GLint success;
    char infoLog[512];

    vert_shader = glCreateShader(GL_VERTEX_SHADER);                   check_gl_error();
    c_str = vert_shader_source.c_str();
    glShaderSource(vert_shader, 1, &c_str, NULL);                     check_gl_error();
    glCompileShader(vert_shader);                                     check_gl_error();

    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);          check_gl_error();
    if(!success) {
        memset(infoLog, 0, 512);
        glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);          check_gl_error();
        nowide::cout << "vert_shader failed\n" << infoLog << std::endl;
    };

    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);                 check_gl_error();
    c_str = frag_shader_source.c_str();
    glShaderSource(frag_shader, 1, &c_str, NULL);                     check_gl_error();
    glCompileShader(frag_shader);                                     check_gl_error();

    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);          check_gl_error();
    if(!success) {
        memset(infoLog, 0, 512);
        glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);          check_gl_error();
        nowide::cout << "frag_shader failed\n" << infoLog << std::endl;
    };

    shader_program = glCreateProgram();                               check_gl_error();
    glAttachShader(shader_program, vert_shader);                      check_gl_error();
    glAttachShader(shader_program, frag_shader);                      check_gl_error();
    glLinkProgram(shader_program);                                    check_gl_error();

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);         check_gl_error();
    if(!success) {
        memset(infoLog, 0, 512);
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);      check_gl_error();
        nowide::cout << "shader program failed\n" << infoLog << std::endl;
    }

    glUseProgram(0);                                                  check_gl_error();
    // glDeleteShader(vert_shader);                                      check_gl_error();
}

void set_inputs(Joypad::button_states_t *buttons, SDL_KeyboardEvent *event) {
    switch(event->keysym.sym) {
    case SDLK_UP:        // DPAD up
        buttons->up     = event->type == SDL_KEYDOWN;
        break;
    case SDLK_DOWN:      // DPAD down
        buttons->down   = event->type == SDL_KEYDOWN;
        break;
    case SDLK_LEFT:      // DPAD left
        buttons->left   = event->type == SDL_KEYDOWN;
        break;
    case SDLK_RIGHT:     // DPAD right
        buttons->right  = event->type == SDL_KEYDOWN;
        break;
    case SDLK_z:         // A button
        buttons->a      = event->type == SDL_KEYDOWN;
        break;
    case SDLK_x:         // B button
        buttons->b      = event->type == SDL_KEYDOWN;
        break;
    case SDLK_RETURN:    // Start button
        buttons->start  = event->type == SDL_KEYDOWN;
        break;
    case SDLK_BACKSPACE: // Select Button
        buttons->select = event->type == SDL_KEYDOWN;
        break;
    }
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_GLContext context;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    // Set the desired OpenGL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create the window using SDL
    window = SDL_CreateWindow("SilverGB",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GB_S_W * 2, GB_S_H * 2, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    // Create the OpenGL context for the window using SDL
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        nowide::cout << SDL_GetError() << std::endl;
        return -1;
    }

    if (gl3wInit() < 0) {
        nowide::cout << "gl3w failed" << std::endl;
        return -1;
    }

    struct Vertex {
        float pos[2];
        float tex[2];
    } vert_data[4]{
        {{-1.0, -1.0},{0.0,0.0}},
        {{-1.0 , 1.0},{1.0,0.0}},
        {{ 1.0, -1.0},{0.0,1.0}},
        {{ 1.0,  1.0},{1.0,1.0}},
    };

    gl_init_textures();
    gl_init_shaders();

    // Set some OpenGL settings
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                             check_gl_error();

    GLuint vert_arr_obj, vert_buff_obj;

    glGenVertexArrays(1, &vert_arr_obj);                              check_gl_error();
    glGenBuffers(1, &vert_buff_obj);                                  check_gl_error();
    glBindVertexArray(vert_arr_obj);                                  check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vert_buff_obj);                     check_gl_error();
    glBufferData(GL_ARRAY_BUFFER,
            sizeof(vert_data),
            vert_data,
            GL_STATIC_DRAW);                                          check_gl_error();

    glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            (void*)0);                                                check_gl_error();
    glEnableVertexAttribArray(0);                                     check_gl_error();
    glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            (void*)(2 * sizeof(float)));                              check_gl_error();
    glEnableVertexAttribArray(1);                                     check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vert_buff_obj);                     check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Silver::File *bios_file = Silver::File::openFile("/home/johna/Documents/SilverGB/test_files/bootroms/cgb_boot.bin");
    Silver::File *rom_file = nullptr;
    Silver::Core *core = nullptr;
    Silver::AudioManager *audio = nullptr;
    Joypad::button_states_t button_state;

    bool isRunning = true;
    SDL_Event event;
    auto start = std::chrono::steady_clock::now();
    int frames = 0;
    while (isRunning) {
        ++frames;
        auto now = std::chrono::steady_clock::now();
        auto diff = now - start;
        if(diff >= std::chrono::seconds(1)) {
            start = now;
            SDL_SetWindowTitle(window, ("SilverGB - " + std::to_string(frames)).c_str());
            frames = 0;
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                set_inputs(&button_state, &event.key);
                break;
            case SDL_DROPFILE:
                if(core) {
                    delete audio;
                    delete core;
                    delete rom_file;
                }

                nowide::cout << event.drop.file << std::endl;

                rom_file = Silver::File::openFile(event.drop.file);
                core = new Silver::Core(rom_file, bios_file);
                audio = Silver::AudioManager::init_audio(core);
                audio->start_audio();
                break;
            case SDL_QUIT:
                isRunning = false;
                break;
            }
        }

        SDL_GL_MakeCurrent(window, context);
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);                                       check_gl_error();
        glClear(GL_COLOR_BUFFER_BIT);                                 check_gl_error();

        if(core) {
            core->set_input_state(button_state);
            core->tick_frame();

            glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
            glTexSubImage2D(
                    GL_TEXTURE_2D,            // target
                    0,                        // level
                    0,                        // xoffset
                    0,                        // yoffset
                    GB_S_W,                   // width
                    GB_S_H,                   // height
                    GL_RGB,                   // format
                    GL_UNSIGNED_BYTE,         // type
                    core->getScreenBuffer());                             check_gl_error();
            glBindTexture(GL_TEXTURE_2D, 0);                              check_gl_error();

            glActiveTexture(GL_TEXTURE0);                                 check_gl_error();
            glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
            glUseProgram(shader_program);                                 check_gl_error();
            glBindVertexArray(vert_arr_obj);                              check_gl_error();
            glUniform1i(glGetUniformLocation(shader_program, "tex_data"), 0); check_gl_error();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                        check_gl_error();
        }

        // Swap the OpenGL window buffers
        SDL_GL_SwapWindow(window);
    }

    if(core) {
        //cleanup the core
        delete core;
    }

    // Release resources
    delete bios_file;
    delete audio;
    SDL_GL_DeleteContext(context);
    SDL_Quit();

    return 0;
}