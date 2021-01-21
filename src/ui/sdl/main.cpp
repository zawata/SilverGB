#include <iostream>

#include "gb_core/core.hpp"

#include <GL/gl3w.h>

#include <SDL2/SDL.h>

GLuint screen_texture,
       shader_program;

#define check_gl_error() _check_gl_error(__LINE__)
void _check_gl_error(u32 line) {
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "GL" << line << ": " << as_hex(err) << std::endl;
    }
}

void gl_init_textures() {
    //Build Screen Texture, leave unpopulated right now  so we can do partial updates later
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
    std::string vert_shader_source = "                      \
#version 150 core                                           \n\
out vec2 v_tex;                                             \n\
                                                            \n\
const vec2 v_pos[4]=vec2[4](vec2(-1.0, 1.0),                \n\
                            vec2(-1.0,-1.0),                \n\
                            vec2( 1.0, 1.0),                \n\
                            vec2( 1.0,-1.0));               \n\
const vec2 t_pos[4]=vec2[4](vec2(0.0,0.0),                  \n\
                            vec2(0.0,1.0),                  \n\
                            vec2(1.0,0.0),                  \n\
                            vec2(1.0,1.0));                 \n\
                                                            \n\
void main()                                                 \n\
{                                                           \n\
    v_tex=t_pos[gl_VertexID];                               \n\
    gl_Position=vec4(v_pos[gl_VertexID], 0.0, 1.0);         \n\
}";

    std::string frag_shader_source = "                      \
#version 150 core                                           \n\
in vec2 v_tex;                                              \n\
uniform sampler2D texSampler;                               \n\
out vec4 color;                                             \n\
void main()                                                 \n\
{                                                           \n\
    color=texture(texSampler, v_tex);                       \n\
}";

    GLuint vert_shader, frag_shader;
    const char *c_str;
    GLint success;

    vert_shader = glCreateShader(GL_VERTEX_SHADER);                   check_gl_error();
    c_str = vert_shader_source.c_str();
    glShaderSource(vert_shader, 1, &c_str, NULL);                     check_gl_error();
    glCompileShader(vert_shader);                                     check_gl_error();

    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);          check_gl_error();
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);          check_gl_error();
        std::cout << "vert_shader failed\n" << infoLog << std::endl;
    };

    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);                 check_gl_error();
    c_str = frag_shader_source.c_str();
    glShaderSource(frag_shader, 1, &c_str, NULL);                     check_gl_error();
    glCompileShader(frag_shader);                                     check_gl_error();

    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);          check_gl_error();
    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);          check_gl_error();
        std::cout << "frag_shader failed\n" << infoLog << std::endl;
    };

    shader_program = glCreateProgram();                               check_gl_error();
    glAttachShader(shader_program, vert_shader);                      check_gl_error();
    glAttachShader(shader_program, frag_shader);                      check_gl_error();
    glLinkProgram(shader_program);                                    check_gl_error();

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);         check_gl_error();
    if(!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);      check_gl_error();
        std::cout << "shader program failed\n" << infoLog << std::endl;
    }

    glUseProgram(0);                                                  check_gl_error();
    // glDeleteShader(vert_shader);                                      check_gl_error();
    // glDeleteShader(vert_shader);                                      check_gl_error();
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_GLContext context;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << SDL_GetError() << std::endl;
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
        200, 200, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (window == NULL) {
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }

    // Create the OpenGL context for the window using SDL
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }

    if (gl3wInit() < 0) {
        std::cout << "gl3w failed" << std::endl;
        return -1;
    }

    // Application Variables
    bool isRunning = true;

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
            4 * sizeof(GL_FLOAT),
            (void*)0);                                                check_gl_error();
    glEnableVertexAttribArray(0);                                     check_gl_error();
    glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(GL_FLOAT),
            (void*)(2 * sizeof(float)));                              check_gl_error();
    glEnableVertexAttribArray(1);                                     check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vert_buff_obj);                     check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    u8 tsc[160 * 144 * 3] = {0};
    u8 *ptr = tsc;
    int x = 0, y = 0, p = 0;
    for(int i = 0; i < (160 * 144); i++) {
        switch(i % 3) {
        case 0:
            *ptr++ = 0xFF;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            break;
        case 1:
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            break;
        case 2:
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            break;
        }
    }
    ptr = tsc;
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
            ptr);                                                 check_gl_error();
    glBindTexture(GL_TEXTURE_2D, 0);                              check_gl_error();

    //load GBCore
    Silver::File *rom_file = nullptr;
    #ifdef __linux__ 
        rom_file = Silver::File::openFile("/home/zawata/Documents/silvergb/test_files/super-mario-land.gb");
    #elif _WIN32
        rom_file = Silver::File::openFile("C:\\Users\\zawata\\source\\repos\\SilverGB\\test_files\\tetris.gb");
    #else

    #endif

    GB_Core *core = new GB_Core(rom_file, nullptr);

    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            }
        }

        core->tick_frame();
        // core->getByteFromIO(0);

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

        SDL_GL_MakeCurrent(window, context);
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        glViewport(0, 0, w, h);                                       check_gl_error();
        glClear(GL_COLOR_BUFFER_BIT);                                 check_gl_error();

        glActiveTexture(GL_TEXTURE0);                                 check_gl_error();
        glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
        glUseProgram(shader_program);                                 check_gl_error();
        glBindVertexArray(vert_arr_obj);                              check_gl_error();
        glUniform1i(glGetUniformLocation(shader_program, "tex_data"), 0); check_gl_error();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                        check_gl_error();

        // Swap the OpenGL window buffers
        SDL_GL_SwapWindow(window);
    }

    // Release resources
    SDL_GL_DeleteContext(context);
    SDL_Quit();

    return 0;
}