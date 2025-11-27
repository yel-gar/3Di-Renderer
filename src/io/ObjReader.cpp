#include "ObjReader.hpp"

#include "ObjData.hpp"

#include <fstream>
#include <iostream>
#include <math/UVCoord.hpp>
#include <math/Vector3.hpp>
#include <regex>
#include <string>
#include <vector>

namespace di_renderer::io {
    ObjData ObjReader::read_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Can't open file");
        }

        std::string line;
        std::vector<math::Vector3> vertices;
        std::vector<math::UVCoord> texture_vertices;
        std::vector<math::Vector3> normals;
        std::vector<std::vector<core::FaceVerticeData>> faces;
        float x, y, z, u, v; // NOLINT

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::stringstream ss(line);
            std::string word;
            ss >> word;
            if (word == "v") {
                ss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            } else if (word == "vt") {
                ss >> u >> v;
                texture_vertices.emplace_back(u, v);
            } else if (word == "vn") {
                ss >> x >> y >> z;
                normals.emplace_back(x, y, z);
            } else if (word == "f") {
                std::vector<core::FaceVerticeData> face_vertices;
                while (ss >> word) {
                    std::smatch match;
                    if (!std::regex_match(word, match, FACE_PATTERN)) {
                        throw std::runtime_error("Bad face pattern");
                    }
                    int vertice_index = std::stoi(match[1]) - 1;
                    int texture_index = match[2].matched && !match[2].str().empty() ? std::stoi(match[2]) - 1 : -1;
                    int normal_index = match[3].matched && !match[3].str().empty() ? std::stoi(match[3]) - 1 : -1;

                    face_vertices.push_back({vertice_index, texture_index, normal_index});
                }
                faces.push_back(face_vertices);
            } else if (UNSUPPORTED_LINES.find(word) != UNSUPPORTED_LINES.end()) {
                std::cout << "Skipped unsupported word: " << word << '\n';
            } else {
                throw std::runtime_error("Bad .obj file");
            }
        }

        return {vertices, texture_vertices, normals, faces};
    }
} // namespace di_renderer::io
