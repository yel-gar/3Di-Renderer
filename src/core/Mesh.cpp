#include "Mesh.hpp"

#include <cmath>
#include <limits>
#include <string_view>

namespace di_renderer::core {

    Mesh::Mesh(const std::vector<math::Vector3>& vertices, const std::vector<math::UVCoord>& texture_vertices,
               const std::vector<math::Vector3>& normals,
               const std::vector<std::vector<FaceVerticeData>>& faces) noexcept
        : vertices(vertices), texture_vertices(texture_vertices), normals(normals) {
        triangulate_faces(faces);
        if (this->normals.empty()) {
            compute_vertex_normals();
        }
    }

    Mesh::~Mesh() = default;

    void Mesh::triangulate_faces(const std::vector<std::vector<FaceVerticeData>>& input_faces) noexcept {
        faces.clear();
        faces.reserve(input_faces.size() * 2);

        for (const auto& face : input_faces) {
            if (face.size() < 3) {
                continue;
            }

            if (face.size() == 3) {
                faces.push_back(face);
            } else if (face.size() > 3) {
                for (size_t i = 1; i < face.size() - 1; ++i) {
                    std::vector<FaceVerticeData> triangle;
                    triangle.reserve(3);
                    triangle.push_back(face[0]);
                    triangle.push_back(face[i]);
                    triangle.push_back(face[i + 1]);
                    faces.push_back(std::move(triangle));
                }
            }
        }
    }

    void Mesh::load_texture(std::string_view filename) {
        texture_filename = filename;
    }

    const std::string& Mesh::get_texture_filename() const noexcept {
        return texture_filename;
    }

    void Mesh::compute_vertex_normals() {
        normals.assign(vertices.size(), math::Vector3(0.0F, 0.0F, 0.0F));

        if (vertices.empty() || faces.empty()) {
            return;
        }

        for (const auto& face : faces) {
            if (face.size() < 3) {
                continue;
            }

            int v0_idx = face[0].vi;
            int v1_idx = face[1].vi;
            int v2_idx = face[2].vi;

            if (v0_idx < 0 || v1_idx < 0 || v2_idx < 0 || v0_idx >= static_cast<int>(vertices.size()) ||
                v1_idx >= static_cast<int>(vertices.size()) || v2_idx >= static_cast<int>(vertices.size())) {
                continue;
            }

            const math::Vector3& v0 = vertices[v0_idx];
            const math::Vector3& v1 = vertices[v1_idx];
            const math::Vector3& v2 = vertices[v2_idx];

            const math::Vector3 edge1 = v1 - v0;
            const math::Vector3 edge2 = v2 - v0;
            const math::Vector3 face_normal = edge1.cross(edge2);

            normals[v0_idx] = normals[v0_idx] + face_normal;
            normals[v1_idx] = normals[v1_idx] + face_normal;
            normals[v2_idx] = normals[v2_idx] + face_normal;
        }

        for (auto& normal : normals) {
            float length = normal.length();
            if (length > std::numeric_limits<float>::epsilon()) {
                normal = normal.normalized();
            }
        }
    }

} // namespace di_renderer::core
