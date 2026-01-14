#include "Mesh.hpp"

#include <cmath>
#include <limits>
#include <string_view>

namespace di_renderer::core {

    Mesh::Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
               std::vector<math::Vector3> normals, const std::vector<std::vector<FaceVerticeData>>& faces) noexcept
        : vertices(std::move(vertices)), texture_vertices(std::move(texture_vertices)), normals(std::move(normals)) {
        triangulate_faces(faces);
        if (this->normals.empty()) {
            compute_vertex_normals();
        }
    }

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
                    faces.push_back({face[0], face[i], face[i + 1]});
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
        normals.assign(vertices.size(), math::Vector3(0.0f, 0.0f, 0.0f));

        if (vertices.empty() || faces.empty()) {
            return;
        }

        for (const auto& face : faces) {
            if (face.size() < 3) {
                continue;
            }

            const int v0_idx = face[0].vi;
            const int v1_idx = face[1].vi;
            const int v2_idx = face[2].vi;

            const std::size_t vs = vertices.size();
            if (v0_idx < 0 || v1_idx < 0 || v2_idx < 0 || static_cast<std::size_t>(v0_idx) >= vs ||
                static_cast<std::size_t>(v1_idx) >= vs || static_cast<std::size_t>(v2_idx) >= vs) {
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
            const float length = normal.length();
            if (length > std::numeric_limits<float>::epsilon()) {
                normal = normal.normalized();
            }
        }
    }

    math::Transform& Mesh::get_transform() noexcept {
        return m_transform;
    }

} // namespace di_renderer::core
