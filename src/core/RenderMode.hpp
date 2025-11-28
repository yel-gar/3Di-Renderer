#pragma once
#include <cstdint>

namespace di_renderer::core {
    enum class RenderMode : std::uint8_t {
        POLYGON = 0,
        TEXTURE = 1,
        LIGHTING = 2,
    };
}
