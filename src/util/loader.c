#include "loader.h"

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <stb_image.h>

#define N_POSITIONS 3
#define N_TCOORDS 2
#define N_NORMALS 3

vao_t* loader_load_vao(const float* positions, const float* tcoords, const float* normals, const unsigned int* indices, unsigned int n_vertices, unsigned int n_indices) {
    // Allocate objects
    vao_t* vao = malloc(sizeof(vao_t));
    GLuint buffers[2];
    glGenBuffers(2, buffers);
    glGenVertexArrays(1, &(vao->array));
    glBindVertexArray(vao->array);
    vao->vertex_buffer = buffers[0];
    vao->index_buffer = buffers[1];

    // Calculate stride
    unsigned int stride = 0;
    if (positions) stride += N_POSITIONS;
    if (tcoords) stride += N_TCOORDS;
    if (normals) stride += N_NORMALS;

    // Pack vertex data
    unsigned int tcoords_offset = 0;
    unsigned int normals_offset = 0;
    float* vertex_data = malloc(n_vertices * stride * sizeof(float));
    if (positions) {
        tcoords_offset += N_POSITIONS;
        normals_offset += N_POSITIONS;
        for (int i = 0; i < n_vertices; i++) {
            vertex_data[i * stride] = positions[i * N_POSITIONS];
            vertex_data[i * stride + 1] = positions[i * N_POSITIONS + 1];
            vertex_data[i * stride + 2] = positions[i * N_POSITIONS + 2];
        }
    }
    if (tcoords) {
        normals_offset += N_TCOORDS;
        for (int i = 0; i < n_vertices; i++) {
            vertex_data[i * stride + tcoords_offset] = tcoords[i * N_TCOORDS];
            vertex_data[i * stride + tcoords_offset + 1] = tcoords[i * N_TCOORDS + 1];
        }
    }
    if (normals) {
        for (int i = 0; i < n_vertices; i++) {
            vertex_data[i * stride + normals_offset] = normals[i * N_NORMALS];
            vertex_data[i * stride + normals_offset + 1] = normals[i * N_NORMALS + 1];
            vertex_data[i * stride + normals_offset + 2] = normals[i * N_NORMALS + 2];
        }
    }

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vao->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, n_vertices * stride * sizeof(float), vertex_data, GL_STATIC_DRAW);
    free(vertex_data);

    // Set vertex attribs
    if (positions) {
        glVertexAttribPointer(0, N_POSITIONS, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
    if (tcoords) {
        glVertexAttribPointer(1, N_TCOORDS, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(tcoords_offset * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    if (normals) {
        glVertexAttribPointer(2, N_NORMALS, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(normals_offset * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    vao->vertex_count = n_indices;

    return vao;
}

texture_t* loader_load_texture(const char* path) {
    // stb load
    int width;
    int height;
    int comp;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(path, &width, &height, &comp, 4);

    // Allocate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // stb free
    stbi_image_free(data);

    // Return texture info
    texture_t* texture_struct = malloc(sizeof(texture_t));
    texture_struct->texture = texture;
    return texture_struct;
}

cubemap_t* loader_load_cubemap(const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back) {
    cubemap_t* cubemap = malloc(sizeof(cubemap));
    const char* face_paths[6] = { right, left, top, bottom, front, back };

    // Allocate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    // Load texture
    for (unsigned int i = 0; i < 6; i++) {
        int width;
        int height;
        int comp;
        stbi_set_flip_vertically_on_load(0);
        stbi_uc* data = stbi_load(face_paths[i], &width, &height, &comp, 4);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Params
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cubemap->texture = texture;
    return cubemap;
}

fbo_t* loader_load_framebuffer(int width, int height) {
    fbo_t* fbo = malloc(sizeof(fbo_t));
    glGenFramebuffers(1, &(fbo->framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
    fbo->width = width;
    fbo->height = height;

    // Color texture
    glGenTextures(1, &(fbo->color_texture));
    glBindTexture(GL_TEXTURE_2D, fbo->color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture, 0);

    // Depth texture
    glGenTextures(1, &(fbo->depth_texture));
    glBindTexture(GL_TEXTURE_2D, fbo->depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->depth_texture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo;
}

ssbo_t* loader_load_ssbo(const void* data, unsigned int size) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ssbo_t* ssbo = malloc(sizeof(ssbo_t));
    ssbo->ssbo = buffer;
    ssbo->size = size;
    return ssbo;
}

void loader_resize_framebuffer(fbo_t* fbo, int width, int height) {
    // Don't resize a framebuffer that matches the desired size
    if (width == fbo->width && height == fbo->height) {
        return;
    }
    fbo->width = width;
    fbo->height = height;

    // Delete the old framebuffer minus the free
    GLuint textures[] = { fbo->color_texture, fbo->depth_texture };
    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &(fbo->framebuffer));

    // Create a new framebuffer minus the malloc
    glGenFramebuffers(1, &(fbo->framebuffer));
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);
    glGenTextures(1, &(fbo->color_texture));
    glBindTexture(GL_TEXTURE_2D, fbo->color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture, 0);
    glGenTextures(1, &(fbo->depth_texture));
    glBindTexture(GL_TEXTURE_2D, fbo->depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbo->depth_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void loader_update_ssbo(ssbo_t* ssbo, const void* data, unsigned int size) {
    if (size > ssbo->size) {
        size = ssbo->size;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo->ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p, data, size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void loader_get_ssbo(ssbo_t* ssbo, void* buffer, unsigned int size) {
    // SLOW AS FUCK!!!!
    // ONLY USE FOR DEBUG!!!
    if (size < ssbo->size) {
        size = ssbo->size;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo->ssbo);
    GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(buffer, p, size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void loader_free_vao(vao_t* vao) {
    // Free objects
    GLuint buffers[] = { vao->vertex_buffer, vao->index_buffer };
    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &(vao->array));
    free(vao);
}

void loader_free_texture(texture_t* texture) {
    glDeleteTextures(1, &(texture->texture));
    free(texture);
}

void loader_free_cubemap(cubemap_t* cubemap) {
    glDeleteTextures(1, &(cubemap->texture));
    free(cubemap);
}

void loader_free_framebuffer(fbo_t* fbo) {
    GLuint textures[] = { fbo->color_texture, fbo->depth_texture };
    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &(fbo->framebuffer));
    free(fbo);
}

void loader_free_ssbo(ssbo_t* ssbo) {
    glDeleteBuffers(1, &(ssbo->ssbo));
    free(ssbo);
}