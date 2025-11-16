#pragma once

#include <array>
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

  // <-- declaration only (note the trailing semicolon)
  Mesh(std::vector<math::Vector3> vertices,
       std::vector<math::UVCoord> texture_vertices,
       std::vector<math::Vector3> normals,
       std::vector<std::vector<std::array<int, 3>>> faces) noexcept;

  ~Mesh();

  // ...
};

} // namespace di_renderer::core
