#pragma once

#include "FaceVerticeData.hpp"
#include "math/Transform.hpp"
#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace di_renderer::core {

    class Mesh {
      public:
        using FaceTriangle = std::array<int, 3>;
        using Faces = std::vector<std::vector<FaceVerticeData>>;
        using TriangulatedFaces = std::vector<FaceTriangle>;

        std::vector<math::Vector3> vertices;
        std::vector<math::UVCoord> texture_vertices;
        std::vector<math::Vector3> normals;
        TriangulatedFaces triangulated_faces;
        Faces faces;

        std::string texture_filename;

        Mesh() = default;

        Mesh(std::vector<math::Vector3> vertices, std::vector<math::UVCoord> texture_vertices,
             std::vector<math::Vector3> normals, const std::vector<std::vector<FaceVerticeData>>& faces) noexcept;

        std::size_t vertex_count() const noexcept {
            return vertices.size();
        }
        std::size_t texcoord_count() const noexcept {
            return texture_vertices.size();
        }
        std::size_t normal_count() const noexcept {
            return normals.size();
        }
        std::size_t face_count() const noexcept {
            return triangulated_faces.size();
        }

        void load_texture(const std::string& filename);
        const std::string& get_texture_filename() const noexcept;

        void compute_vertex_normals();

        math::Transform& get_transform() noexcept;

      private:
        math::Transform m_transform;

        void triangulate_faces(const std::vector<std::vector<FaceVerticeData>>& input_faces) noexcept;
    };

} // namespace di_renderer::core
