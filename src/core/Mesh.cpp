#include "core/Mesh.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace di_renderer::core {

    Mesh::Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
               std::vector<math::Vector3> normals, std::vector<std::vector<FaceVerticeData>> faces) noexcept
        : vertices(std::move(vertices)), texture_vertices(std::move(texture_vertices)), normals(std::move(normals)),
          faces(std::move(faces)) {}

    Mesh::~Mesh() = default;

    void Mesh::load_texture(const std::string& filename) {
        std::cout << "Loading " << filename << '\n';
        texture_filename = filename;
        throw std::runtime_error("Not implemented yet!");
    }

    const std::string& Mesh::get_texture_filename() const noexcept {
        return texture_filename;
    }
} // namespace di_renderer::core
