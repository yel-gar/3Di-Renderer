#include "UVCoord.hpp"

namespace di_renderer::math {
    UVCoord::UVCoord() : u(0.0F), v(0.0F) {}

    UVCoord::UVCoord(const float u, const float v) : u(u), v(v) {}
} // namespace di_renderer::math
