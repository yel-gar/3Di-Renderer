#pragma once

#include <array>
#include <cstddef>
#include <epoxy/gl.h>

namespace di_renderer::graphics {

    struct Vertex {
        std::array<float, 3> position;
        std::array<float, 3> color;
        std::array<float, 3> normal;
        std::array<float, 2> uv;
    };

    GLuint create_shader_program();
    void destroy_shader_program(GLuint program);

    void init_mesh_batch();
    void destroy_mesh_batch();

    void draw_indexed_mesh(const Vertex* vertices, size_t vertex_count, const unsigned int* indices, size_t index_count,
                           GLuint shader_program);

} // namespace di_renderer::graphics
