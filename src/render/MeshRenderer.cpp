#include "MeshRenderer.hpp"

#include "Vertex.hpp"
#include "core/Mesh.hpp"

#include <epoxy/gl.h>
#include <vector>

namespace di_renderer::render {

    MeshRenderer::MeshRenderer() = default;
    MeshRenderer::~MeshRenderer() {
        cleanup();
    }

    bool MeshRenderer::initialize(const core::Mesh& mesh) {
        cleanup(); // Clean up any existing resources

        // Convert mesh to vertex array format
        std::vector<Vertex> vertices;
        vertices.reserve(mesh.faces.size() * 3); // Each face has at least 3 vertices after triangulation

        for (const auto& face : mesh.faces) {
            if (face.size() < 3)
                continue;

            for (size_t i = 0; i < face.size() - 2; ++i) {
                // Process each triangle in the face
                const auto& v0_data = face[0];
                const auto& v1_data = face[i + 1];
                const auto& v2_data = face[i + 2];

                // Create vertices for the triangle
                for (const auto& vert_data : {v0_data, v1_data, v2_data}) {
                    if (static_cast<size_t>(vert_data.vi) < mesh.vertices.size()) {
                        Vertex vertex;

                        // Set position
                        const auto& pos = mesh.vertices[vert_data.vi];
                        vertex.position = {pos.x, pos.y, pos.z};

                        // Set default color (white)
                        vertex.color = {1.0f, 1.0f, 1.0f};

                        // Set normal (use vertex normal if available)
                        if (static_cast<size_t>(vert_data.vi) < mesh.normals.size()) {
                            const auto& normal = mesh.normals[vert_data.vi];
                            vertex.normal = {normal.x, normal.y, normal.z};
                        } else {
                            vertex.normal = {0.0f, 0.0f, 1.0f}; // Default normal
                        }

                        // Set UV coordinates (use texture vertex if available)
                        if (vert_data.ti >= 0 && static_cast<size_t>(vert_data.ti) < mesh.texture_vertices.size()) {
                            const auto& uv = mesh.texture_vertices[vert_data.ti];
                            vertex.uv = {uv.u, uv.v};
                        } else {
                            vertex.uv = {0.0f, 0.0f}; // Default UV
                        }

                        vertices.push_back(vertex);
                    }
                }
            }
        }

        if (vertices.empty()) {
            return false; // No valid vertices to render
        }

        // Generate VAO
        glGenVertexArrays(1, &vao_);
        glBindVertexArray(vao_);

        // Create and bind the vertex buffer
        glGenBuffers(1, &vbo_vertices_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // Set up vertex attributes
        // Position attribute (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(0);

        // Color attribute (location 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, color)));
        glEnableVertexAttribArray(1);

        // Normal attribute (location 2)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, normal)));
        glEnableVertexAttribArray(2);

        // UV attribute (location 3)
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
        glEnableVertexAttribArray(3);

        glBindVertexArray(0);

        vertex_count_ = static_cast<GLsizei>(vertices.size());

        return true;
    }

    void MeshRenderer::render(GLuint shader_program) {
        if (vao_ == 0 || vertex_count_ == 0)
            return;

        glBindVertexArray(vao_);
        glUseProgram(shader_program);

        // Draw the mesh as triangles
        glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

        glBindVertexArray(0);
    }

    void MeshRenderer::cleanup() {
        if (vao_) {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
        if (vbo_vertices_) {
            glDeleteBuffers(1, &vbo_vertices_);
            vbo_vertices_ = 0;
        }
        if (ebo_) {
            glDeleteBuffers(1, &ebo_);
            ebo_ = 0;
        }

        vertex_count_ = 0;
    }

} // namespace di_renderer::render
