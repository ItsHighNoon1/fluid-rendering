#include "shader.h"

#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#include <glad/glad.h>

#include "../util/file.h"

GLuint _allocate_shader(const char* path, GLenum type) {
    char* source = NULL;
    file_read_all(path, &source, NULL);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&source, NULL);
    glCompileShader(shader);
    free(source);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char infoLog[512];
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compile error:\n%s\n", infoLog);
    }

    return shader;
}

shader_t* shader_create(const char* vert_path, const char* frag_path) {
    GLuint vertex = _allocate_shader(vert_path, GL_VERTEX_SHADER);
    GLuint fragment = _allocate_shader(frag_path, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char infoLog[512];
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Program link error:\n%s\n", infoLog);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    shader_t* shader_struct = malloc(sizeof(shader_t));
    shader_struct->program = program;
    return shader_struct;
}

shader_t* shader_create_compute(const char* path) {
    GLuint compute = _allocate_shader(path, GL_COMPUTE_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, compute);
    glLinkProgram(program);
    glDeleteShader(compute);
    shader_t* shader_struct = malloc(sizeof(shader_t));
    shader_struct->program = program;
    return shader_struct;
}

void shader_bind(shader_t* shader) {
    glUseProgram(shader->program);
}

void shader_free(shader_t* shader) {
    glDeleteProgram(shader->program);
    free(shader);
}

GLint uniform_find(shader_t* shader, const char* name) {
    return glGetUniformLocation(shader->program, name);
}

void uniform_mat4(GLint location, mat4 m) {
    glUniformMatrix4fv(location, 1, GL_FALSE, m[0]);
}

void uniform_vec3(GLint location, vec3 v) {
    glUniform3f(location, v[0], v[1], v[2]);
}

void uniform_vec2(GLint location, vec2 v) {
    glUniform2f(location, v[0], v[1]);
}

void uniform_float(GLint location, float f) {
    glUniform1f(location, f);
}

void uniform_int(GLint location, int i) {
    glUniform1i(location, i);
}

GLuint ssbo_find(shader_t* shader, const char* name) {
    return glGetProgramResourceIndex(shader->program, GL_SHADER_STORAGE_BLOCK, name);
}