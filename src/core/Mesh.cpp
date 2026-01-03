#include "Mesh.hpp"

#include <cmath>
#include <iostream>

namespace di_renderer::core {

    Mesh::Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
               std::vector<math::Vector3> normals, const std::vector<std::vector<FaceVerticeData>>& faces) noexcept
        : vertices(std::move(vertices)), texture_vertices(std::move(texture_vertices)), normals(std::move(normals)),
          faces(faces) {
        triangulate_faces(this->faces);
        if (this->normals.empty()) {
            compute_vertex_normals();
        }
    }

    void Mesh::triangulate_faces(const std::vector<std::vector<FaceVerticeData>>& input_faces) noexcept {
        std::size_t expected = 0;
        for (const auto& f : input_faces) {
            if (f.size() >= 3) {
                expected += (f.size() - 2);
            }
        }
        triangulated_faces.clear();
        triangulated_faces.reserve(expected);

        const int vcount = static_cast<int>(vertices.size());

        for (const auto& face : input_faces) {
            const std::size_t n = face.size();
            if (n < 3) {
                continue;
            }

            bool valid_face = true;
            std::vector<int> idxs;
            idxs.reserve(n);
            for (const auto& fv : face) {
                auto vi_signed = static_cast<int64_t>(fv.vi);
                if (vi_signed < 0 || vi_signed >= vcount) {
                    valid_face = false;
                    break;
                }
                idxs.push_back(static_cast<int>(vi_signed));
            }
            if (!valid_face) {
                continue;
            }

            for (std::size_t i = 1; i + 1 < idxs.size(); ++i) {
                triangulated_faces.push_back({idxs[0], idxs[i], idxs[i + 1]});
            }
        }
    }

    void Mesh::load_texture(const std::string& filename) {
        std::cout << "Loading " << filename << '\n';
        texture_filename = filename;
        throw std::runtime_error("Not implemented yet!");
    }

    const std::string& Mesh::get_texture_filename() const noexcept {
        return texture_filename;
    }

    void Mesh::compute_vertex_normals() {
        if (vertices.empty()) {
            normals.clear();
            return;
        }

        normals.assign(vertices.size(), math::Vector3(0.0f, 0.0f, 0.0f));

        if (triangulated_faces.empty()) {
            return;
        }

        const std::size_t vcount = vertices.size();

        for (const auto& tri : triangulated_faces) {
            if (tri[0] < 0 || tri[1] < 0 || tri[2] < 0) {
                continue;
            }
            const auto i0 = static_cast<std::size_t>(tri[0]);
            const auto i1 = static_cast<std::size_t>(tri[1]);
            const auto i2 = static_cast<std::size_t>(tri[2]);

            if (i0 >= vcount || i1 >= vcount || i2 >= vcount) {
                continue;
            }

            const math::Vector3& v0 = vertices[i0];
            const math::Vector3& v1 = vertices[i1];
            const math::Vector3& v2 = vertices[i2];

            const math::Vector3 edge1 = v1 - v0;
            const math::Vector3 edge2 = v2 - v0;
            const math::Vector3 face_normal = edge1.cross(edge2);

            normals[i0] = normals[i0] + face_normal;
            normals[i1] = normals[i1] + face_normal;
            normals[i2] = normals[i2] + face_normal;
        }

        const float eps = 1e-8f;
        for (auto& normal : normals) {
            const float length = normal.length();
            if (length > eps) {
                normal = normal.normalized();
            } else {
                normal = math::Vector3(0.0f, 0.0f, 0.0f);
            }
        }
    }

    math::Transform& Mesh::get_transform() noexcept {
        return m_transform;
    }

} // namespace di_renderer::core
