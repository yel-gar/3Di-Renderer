#include "core/Mesh.hpp"
#include <utility>

namespace di_renderer::core {

Mesh::Mesh(std::vector<math::Vector3> vertices,
           std::vector<math::UVCoord> texture_vertices,
           std::vector<math::Vector3> normals,
           std::vector<std::vector<std::array<int, 3>>> faces) noexcept
    : vertices(std::move(vertices)),
      texture_vertices(std::move(texture_vertices)),
      normals(std::move(normals)), faces(std::move(faces)) {}

Mesh::~Mesh() = default;

} // namespace di_renderer::core
