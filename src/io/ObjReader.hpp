#ifndef DI_RENDERER_OBJREADER_HPP
#define DI_RENDERER_OBJREADER_HPP
#include <regex>

class ObjReader {
public:
    static void read_file(std::string &filename);

private:
    inline static const std::regex face_pattern{R"((\d+)\/?(\d*)\/?(\d*))"};
};

#endif //DI_RENDERER_OBJREADER_HPP
