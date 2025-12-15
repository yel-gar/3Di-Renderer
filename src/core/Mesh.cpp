#include "Mesh.hpp"

#include <cmath>

namespace di_renderer::core {

    Mesh::Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
               std::vector<math::Vector3> normals, const std::vector<std::vector<FaceVerticeData>>& faces) noexcept
        : vertices(std::move(vertices)), texture_vertices(std::move(texture_vertices)), normals(std::move(normals)),
          faces(faces) {
        triangulate_faces(faces);
        if (this->normals.empty()) {
            compute_vertex_normals();
        }
    }

    Mesh::~Mesh() = default;

    void Mesh::triangulate_faces(const std::vector<std::vector<FaceVerticeData>>& input_faces) noexcept {
        triangulated_faces.reserve(input_faces.size() * 2);

        for (const auto& face : input_faces) {
            if (face.size() < 3) {
                continue;
            }

            if (face.size() == 3) {
                triangulated_faces.push_back({{face[0].vi, face[1].vi, face[2].vi}});
            } else if (face.size() > 3) {
                for (size_t i = 1; i < face.size() - 1; ++i) {
                    triangulated_faces.push_back({{face[0].vi, face[i].vi, face[i + 1].vi}});
                }
            }
        }
    }

    void Mesh::load_texture(const std::string& filename) {
        texture_filename = filename;
    }

    const std::string& Mesh::get_texture_filename() const noexcept {
        return texture_filename;
    }

    void Mesh::compute_vertex_normals() {
        normals.assign(vertices.size(), math::Vector3(0.0F, 0.0F, 0.0F));

        if (vertices.empty() || triangulated_faces.empty()) {
            return;
        }

        for (const auto& tri : triangulated_faces) {
            if (tri[0] >= static_cast<int>(vertices.size()) || tri[1] >= static_cast<int>(vertices.size()) ||
                tri[2] >= static_cast<int>(vertices.size())) {
                continue;
            }

            const math::Vector3& v0 = vertices[tri[0]];
            const math::Vector3& v1 = vertices[tri[1]];
            const math::Vector3& v2 = vertices[tri[2]];

            math::Vector3 edge1 = v1 - v0;
            math::Vector3 edge2 = v2 - v0;
            math::Vector3 face_normal = edge1.cross(edge2);

            normals[tri[0]] = normals[tri[0]] + face_normal;
            normals[tri[1]] = normals[tri[1]] + face_normal;
            normals[tri[2]] = normals[tri[2]] + face_normal;
        }

        for (auto& normal : normals) {
            float length = normal.length();
            if (length > 1e-8F) {
                normal = normal.normalized();
            }
        }
    }

} // namespace di_renderer::core
