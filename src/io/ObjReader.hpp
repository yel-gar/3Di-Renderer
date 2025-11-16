#pragma once
#include <set>
#include <regex>
#include "ObjData.hpp"

namespace di_renderer::io {
    class ObjReader {
    public:
        static ObjData read_file(const std::string &filename);

    private:
        inline static const std::regex face_pattern{R"((\d+)\/?(\d*)\/?(\d*))"};
        inline static const std::set<std::string> unsupported_lines{"o", "g", "s", "usemtl", "mtllib", "vp"};
    };
}