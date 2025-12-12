#pragma once
#include "core/Mesh.hpp"

#include <string>

namespace di_renderer::io {
    class ObjWriter {
      public:
        static void write_file(const std::string& filename, const core::Mesh& mesh);
    };
} // namespace di_renderer::io
