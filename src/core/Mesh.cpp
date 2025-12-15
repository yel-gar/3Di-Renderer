#include "Mesh.hpp"

namespace di_renderer::core {

    Mesh::Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
               std::vector<math::Vector3> normals, const std::vector<std::vector<FaceVerticeData>>& faces) noexcept
        : vertices(std::move(vertices)), texture_vertices(std::move(texture_vertices)), normals(std::move(normals)) {

        triangulate_faces(faces);
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

} // namespace di_renderer::core
