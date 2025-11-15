#include "ObjReader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>


void ObjReader::read_file(std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Can't open file");
    }

    std::string line;
    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<float, 2>> texture_vertices;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::vector<std::array<int, 3>>> faces;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string word;
        ss >> word;
        if (word == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.push_back({x, y, z});
        } else if (word == "vt") {
            float u, v;
            ss >> u >> v;
            texture_vertices.push_back({u, v});
        } else if (word == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            normals.push_back({x, y, z});
        } else if (word == "f") {
            std::vector<std::array<int, 3> > face_vertices;
            while (ss >> word) {
                std::smatch match;
                if (!std::regex_match(word, match, face_pattern)) {
                    throw std::runtime_error("Bad face pattern");
                }
                int vertice_index = std::stoi(match[1]) - 1;
                int texture_index = match[2].matched && !match[2].str().empty() ? std::stoi(match[2]) - 1 : -1;
                int normal_index = match[3].matched && !match[3].str().empty() ? std::stoi(match[3]) - 1 : -1;

                face_vertices.push_back({vertice_index, texture_index, normal_index});
            }
            faces.push_back(face_vertices);
        } else {
            throw std::runtime_error("Bad .obj file");
        }
    }
}
