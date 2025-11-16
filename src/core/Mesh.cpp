#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::core {
class Mesh {
public:
  using FaceTriangle = std::array<int, 3>;
  using Face = std::vector<FaceTriangle>;
  using Faces = std::vector<Face>;

  std::vector<math::Vector3> vertices;
  std::vector<math::UVCoord> texture_vertices;
  std::vector<math::Vector3> normals;
  Faces faces;

  Mesh() = default;

  Mesh(std::vector<math::Vector3> vertices,
       std::vector<math::UVCoord> texture_vertices,
       std::vector<math::Vector3> normals,
       std::vector<std::vector<std::array<int, 3>>> faces) noexcept;

  ~Mesh();

  Mesh(const Mesh &) = default;
  Mesh(Mesh &&) noexcept = default;
  Mesh &operator=(const Mesh &) = default;
  Mesh &operator=(Mesh &&) noexcept = default;

  std::size_t vertex_count() const noexcept { return vertices.size(); }
  std::size_t texcoord_count() const noexcept {
    return texture_vertices.size();
  }
  std::size_t normal_count() const noexcept { return normals.size(); }
  std::size_t face_count() const noexcept { return faces.size(); }
};

} // namespace di_renderer::core
