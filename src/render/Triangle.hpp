#pragma once

#include <array>
#include <cstddef>
#include <epoxy/gl.h>

namespace di_renderer {
    namespace graphics {

        struct Vertex {
            std::array<float, 3> position;
            std::array<float, 3> color;
            std::array<float, 3> normal;
            std::array<float, 2> uv;
        };

        GLuint create_shader_program();
        void destroy_shader_program(GLuint program);

        void init_triangle_batch();
        void destroy_triangle_batch();

        void draw_single_triangle(const Vertex* vertices, GLuint shader_program);
        void draw_triangle_batch(const Vertex* vertices, size_t count, GLuint shader_program);

    } // namespace graphics
} // namespace di_renderer
