#pragma once

#include <array>

namespace di_renderer::render {
    struct Vertex {
        std::array<float, 3> position; // x, y, z
        std::array<float, 3> color;    // r, g, b
        std::array<float, 3> normal;   // nx, ny, nz
        std::array<float, 2> uv;       // u, v
    };
} // namespace di_renderer::render
