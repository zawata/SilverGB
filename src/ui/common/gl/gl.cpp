#include <cstring>
#include <cstdarg>

#include "gl.hpp"

void _check_gl_error(unsigned int line) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "GL-" << line << ": " << std::hex << err << std::dec << std::endl;
    }
}

// It seems like glad doesn't supply a gles loader but it supply a gl loader.
// just gonna give it one anyways to keep the code cleaner
void init(void *(*loaderProc)(const char *)) {
	glGetString = (PFNGLGETSTRINGPROC)loaderProc("glGetString");
	if(glGetString == NULL) std::cout << "no func" << std::endl;
	if(glGetString(GL_VERSION) == NULL) std::cout << "bad func" << std::endl;

#if defined(__EMSCRIPTEN__)
    if(!gladLoadGLES2Loader(loaderProc)) {
#else
    if(!gladLoadGLLoader(loaderProc)) {
#endif
        std::cout << "glad failed" << std::endl;
    }

}

GLuint initialize_empty_texture(int width, int height) {
    // TODO: check for GL_OES_texture_border_clamp

    GLuint tex_id;
    //Build Screen Texture, leave unpopulated right now so we can do partial updates later
    glGenTextures(1, &tex_id);                                                  check_gl_error();
    glBindTexture(GL_TEXTURE_2D, tex_id);                                       check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);           check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);           check_gl_error();
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);      check_gl_error();
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);      check_gl_error();
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);                   check_gl_error();

    //unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);                                            check_gl_error();

    return tex_id;
}

static GLuint compile_shader(GLuint type, const char *shader_source);

GLuint compile_vertex_shader(const char *shader_source) {
    return compile_shader(GL_VERTEX_SHADER, shader_source);
}

GLuint compile_fragment_shader(const char *shader_source) {
    return compile_shader(GL_FRAGMENT_SHADER, shader_source);
}

static GLuint compile_shader(GLuint type, const char *shader_source) {
    GLuint shader;
    GLint success;
    char infoLog[512];

    shader = glCreateShader(type);                                              check_gl_error();
    glShaderSource(shader, 1, &shader_source, NULL);                            check_gl_error();
    glCompileShader(shader);                                                    check_gl_error();

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);                         check_gl_error();
    if(!success) {
        memset(infoLog, 0, 512);
        glGetShaderInfoLog(shader, 512, NULL, infoLog);                         check_gl_error();
        std::cerr << "shader failed\n" << infoLog << std::endl;
        return -1;
    };

    return shader;
}

GLuint link_shader_program(int count, ...) {
    va_list args;
    va_start(args, count);

    GLint success;
    char infoLog[512];

    GLuint shader_program = glCreateProgram();                                  check_gl_error();
    for(int i = 0; i < count; i++) {
        glAttachShader(shader_program, va_arg(args, GLuint));                              check_gl_error();
    }
    glLinkProgram(shader_program);                                              check_gl_error();
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);                   check_gl_error();
    if(!success) {
        memset(infoLog, 0, 512);
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);                check_gl_error();
        std::cerr << "shader program failed\n" << infoLog << std::endl;
    }

    //unbind current program
    glUseProgram(0);                                                            check_gl_error();

    va_end(args);
    return shader_program;
}

void initialize_vertex_arrays(GLuint *vert_arr_obj, GLuint *vert_buff_obj) {
    struct Vertex {
        float pos[2];
        float tex[2];
    } vert_data[4]{
        {{-1.0, -1.0},{0.0,0.0}},
        {{-1.0 , 1.0},{1.0,0.0}},
        {{ 1.0, -1.0},{0.0,1.0}},
        {{ 1.0,  1.0},{1.0,1.0}},
    };

    glGenVertexArrays(1, vert_arr_obj);                                         check_gl_error();
    glGenBuffers(1, vert_buff_obj);                                             check_gl_error();
    glBindVertexArray(*vert_arr_obj);                                           check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, *vert_buff_obj);                              check_gl_error();
    glBufferData(GL_ARRAY_BUFFER,
            sizeof(vert_data),
            vert_data,
            GL_STATIC_DRAW);                                                    check_gl_error();

    glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            (void*)0);                                                          check_gl_error();
    glEnableVertexAttribArray(0);                                               check_gl_error();
    glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            (void*)(2 * sizeof(float)));                                        check_gl_error();
    glEnableVertexAttribArray(1);                                               check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, *vert_buff_obj);                              check_gl_error();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void begin_draw(int w, int h) {
    // TODO: do better with stretching
    glViewport(0, 0, w, h);                                                     check_gl_error();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                                       check_gl_error();
    glClear(GL_COLOR_BUFFER_BIT);                                               check_gl_error();
}

void draw_screen_to_texture(GLuint texture, int width, int height, const uint8_t *buffer) {
    glBindTexture(GL_TEXTURE_2D, texture);                                      check_gl_error();
    glTexSubImage2D(
                    GL_TEXTURE_2D,            // target
                    0,                        // level
                    0,                        // xoffset
                    0,                        // yoffset
                    width,                    // width
                    height,                   // height
                    GL_RGB,                   // format
                    GL_UNSIGNED_BYTE,         // type
                    buffer);                                                    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, 0);                                            check_gl_error();
}

void draw_display_quad(GLuint vert_arr_obj, GLuint screen_texture, GLuint shader_program) {
    glActiveTexture(GL_TEXTURE0);                                 check_gl_error();
    glBindTexture(GL_TEXTURE_2D, screen_texture);                 check_gl_error();
    glUseProgram(shader_program);                                 check_gl_error();
    glBindVertexArray(vert_arr_obj);                              check_gl_error();
    glUniform1i(glGetUniformLocation(shader_program, "tex_data"), 0); check_gl_error();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);                        check_gl_error();
}
