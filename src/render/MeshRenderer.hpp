#pragma once

#include "core/Mesh.hpp"

#include <epoxy/gl.h> // For GLuint and OpenGL types
#include <memory>

namespace di_renderer::render {

    class MeshRenderer {
      public:
        MeshRenderer();
        ~MeshRenderer();

        // Initialize the renderer with mesh data
        bool initialize(const core::Mesh& mesh);

        // Render the mesh using the specified shader program
        void render(GLuint shader_program);

        // Cleanup OpenGL resources
        void cleanup();

        // Check if the renderer is properly initialized
        bool is_initialized() const noexcept {
            return vao_ != 0;
        }

      private:
        GLuint vao_ = 0;
        GLuint vbo_vertices_ = 0; // Combined vertex buffer containing position, color, normal, uv
        GLuint ebo_ = 0;
        GLsizei vertex_count_ = 0;
    };

} // namespace di_renderer::render
