#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>

#define check_gl_error() _check_gl_error(__LINE__)
void _check_gl_error(unsigned int line);

void init(void *(*loaderProc)(const char *));
GLuint initialize_empty_texture(int width, int height);
GLuint compile_vertex_shader(const char *shader_source);
GLuint compile_fragment_shader(const char *shader_source);
GLuint link_shader_program(int count, ...);
void initialize_vertex_arrays(GLuint *vert_arr_obj, GLuint *vert_buff_obj);

void begin_draw(int w, int h);
void draw_screen_to_texture(GLuint texture, int width, int height, const uint8_t *buffer);
void draw_display_quad(GLuint vert_arr_obj, GLuint screen_texture, GLuint shader_program);
