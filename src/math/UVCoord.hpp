#pragma once

namespace di_renderer::math {
    class UVCoord {
    public:
        float u;
        float v;

        UVCoord();
        UVCoord(float u, float v);
    };
}
