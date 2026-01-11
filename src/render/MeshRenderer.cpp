#include "MeshRenderer.hpp"

#include <cmath>
#include <vector>

namespace di_renderer::graphics {

    void draw_mesh(const di_renderer::core::Mesh& mesh, GLuint shader_program) {
        if (mesh.vertices.empty()) {
            return;
        }

        std::vector<Vertex> vertices;
        vertices.reserve(mesh.vertices.size());

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            Vertex vertex;
            vertex.position = {mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z};

            if (i < mesh.normals.size()) {
                vertex.normal = {mesh.normals[i].x, mesh.normals[i].y, mesh.normals[i].z};
            } else {
                vertex.normal = {0.0f, 1.0f, 0.0f};
            }

            if (i < mesh.texture_vertices.size()) {
                vertex.uv = {mesh.texture_vertices[i].u, mesh.texture_vertices[i].v};
            } else {
                vertex.uv = {0.0f, 0.0f};
            }

            // Simple color based on position
            vertex.color = {(std::sin(mesh.vertices[i].x * 2.0f) + 1.0f) * 0.5f,
                            (std::sin(mesh.vertices[i].y * 2.0f) + 1.0f) * 0.5f,
                            (std::sin(mesh.vertices[i].z * 2.0f) + 1.0f) * 0.5f};

            vertices.push_back(vertex);
        }

        draw_triangle_batch(vertices.data(), vertices.size(), shader_program);
    }

} // namespace di_renderer::graphics
