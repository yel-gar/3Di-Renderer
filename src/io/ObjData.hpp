#pragma once
#include <array>
#include <vector>

#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::io {
    struct ObjData {
        std::vector<math::Vector3> vertices;
        std::vector<math::UVCoord> texture_vertices;
        std::vector<math::Vector3> normals;
        std::vector<std::vector<std::array<int, 3>>> faces;
    };
}
