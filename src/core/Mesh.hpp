#pragma once

#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace di_renderer::core {

    class Mesh {
      public:
        using Triangle = std::array<int, 3>;

        std::vector<math::Vector3> vertices;
        std::vector<math::UVCoord> texture_vertices;
        std::vector<math::Vector3> normals;
        std::vector<Triangle> triangles;

        Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
             std::vector<math::Vector3> normals, std::vector<std::vector<std::array<int, 3>>> faces);

        Mesh() = default;

        ~Mesh() = default;

        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) noexcept = default;
        Mesh& operator=(const Mesh&) = default;
        Mesh& operator=(Mesh&&) noexcept = default;

        std::size_t vertex_count() const noexcept {
            return vertices.size();
        }
        std::size_t texcoord_count() const noexcept {
            return texture_vertices.size();
        }
        std::size_t normal_count() const noexcept {
            return normals.size();
        }
        std::size_t triangle_count() const noexcept {
            return triangles.size();
        }

      private:
        std::vector<Triangle> triangulate_face(const std::vector<std::array<int, 3>>& face) const;
        std::vector<Triangle> triangulate_face_convex_fan(const std::vector<int>& vertex_indices) const;
        std::vector<int> extract_vertex_indices(const std::vector<std::array<int, 3>>& face) const;
    };

} // namespace di_renderer::core
