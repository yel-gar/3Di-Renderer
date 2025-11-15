#pragma once
#include <regex>

namespace di_renderer::io {
    class ObjReader {
    public:
        static void read_file(std::string &filename);

    private:
        inline static const std::regex face_pattern{R"((\d+)\/?(\d*)\/?(\d*))"};
    };
}