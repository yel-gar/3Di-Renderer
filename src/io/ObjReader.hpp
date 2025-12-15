#pragma once
#include "ObjData.hpp"

#include <regex>
#include <unordered_set>

namespace di_renderer::io {
    class ObjReader {
      public:
        static ObjData read_file(const std::string& filename);

      private:
        inline static const std::regex FACE_PATTERN{R"((\d+)\/?(\d*)\/?(\d*))"};
        inline static const std::unordered_set<std::string> UNSUPPORTED_LINES{"o", "g", "s", "usemtl", "mtllib", "vp"};
    };
} // namespace di_renderer::io
