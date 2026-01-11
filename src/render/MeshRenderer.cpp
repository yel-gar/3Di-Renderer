#include "MeshRenderer.hpp"

#include "Triangle.hpp"

#include <cmath>
#include <vector>

namespace di_renderer::graphics {

    void draw_mesh(const di_renderer::core::Mesh& mesh, GLuint shader_program) {
        if (mesh.vertices.empty()) {
            return;
        }

        std::vector<Vertex> vertices;
        vertices.reserve(mesh.vertices.size());

        std::vector<unsigned int> indices;
        indices.reserve(mesh.faces.size() * 3); // Reserve space for triangle indices

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            Vertex vertex;
            vertex.position = {{mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z}};

            if (i < mesh.normals.size()) {
                vertex.normal = {{mesh.normals[i].x, mesh.normals[i].y, mesh.normals[i].z}};
            } else {
                vertex.normal = {{0.0f, 1.0f, 0.0f}};
            }

            if (i < mesh.texture_vertices.size()) {
                vertex.uv = {{mesh.texture_vertices[i].u, mesh.texture_vertices[i].v}};
            } else {
                vertex.uv = {{0.0f, 0.0f}};
            }

            vertex.color = {{0.7f, 0.7f, 0.7f}};

            vertices.push_back(vertex);
        }

        for (const auto& face : mesh.faces) {
            if (face.size() < 3)
                continue;

            if (face.size() == 3) {
                indices.push_back(face[0].vi);
                indices.push_back(face[1].vi);
                indices.push_back(face[2].vi);
            } else {
                for (size_t i = 1; i < face.size() - 1; ++i) {
                    indices.push_back(face[0].vi);
                    indices.push_back(face[i].vi);
                    indices.push_back(face[i + 1].vi);
                }
            }
        }

        if (!indices.empty()) {
            draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(), indices.size(), shader_program);
        }
    }

} // namespace di_renderer::graphics
