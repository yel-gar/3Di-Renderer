#include "Mesh.hpp"
#include <stdexcept>

namespace di_renderer::core {

Mesh::Mesh(std::vector<math::Vector3> vertices,
           std::vector<math::UVCoord> texture_vertices,
           std::vector<math::Vector3> normals,
           std::vector<std::vector<std::array<int, 3>>> faces)
    : vertices(std::move(vertices)),
      texture_vertices(std::move(texture_vertices)),
      normals(std::move(normals)) {
  for (const auto &face : faces) {
    auto face_triangles = triangulate_face(face);
    triangles.insert(triangles.end(), face_triangles.begin(),
                     face_triangles.end());
  }

  if (triangles.empty() && !vertices.empty()) {
    throw std::invalid_argument(
        "Mesh construction resulted in no triangles from non-empty vertices");
  }
}

std::vector<Mesh::Triangle>
Mesh::triangulate_face(const std::vector<std::array<int, 3>> &face) const {
  if (face.size() < 3) {
    return {};
  }

  if (face.size() == 3) {
    return {{{face[0][0], face[1][0], face[2][0]}}};
  }

  std::vector<int> vertex_indices = extract_vertex_indices(face);

  return triangulate_face_convex_fan(vertex_indices);
}

std::vector<int> Mesh::extract_vertex_indices(
    const std::vector<std::array<int, 3>> &face) const {
  std::vector<int> indices;
  indices.reserve(face.size());

  for (const auto &vertex_data : face) {
    indices.push_back(vertex_data[0]);
  }

  return indices;
}

std::vector<Mesh::Triangle> Mesh::triangulate_face_convex_fan(
    const std::vector<int> &vertex_indices) const {
  std::vector<Triangle> triangles;

  if (vertex_indices.size() < 3) {
    return triangles;
  }

  int first_index = vertex_indices[0];
  for (size_t i = 1; i < vertex_indices.size() - 1; ++i) {
    triangles.push_back(
        {first_index, vertex_indices[i], vertex_indices[i + 1]});
  }

  return triangles;
}

} // namespace di_renderer::core
