#pragma once

namespace di_renderer::core {
    enum class RenderMode : std::uint8_t {
        VERTICES = 0,
        POLYGONAL = 1,
        TEXTURE = 2,
    };
}