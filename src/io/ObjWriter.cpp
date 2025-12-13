#include "ObjWriter.hpp"

#include <fstream>

namespace di_renderer::io {
    void ObjWriter::write_file(const std::string& filename, const core::Mesh& mesh) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + filename);
        }
        file << "# Generated with Di-Renderer ObjWriter\n\n";

        // vertices
        for (const auto& v : mesh.vertices) {
            file << "v " << v.x << " " << v.y << " " << v.z << '\n';
        }
        file << '\n';

        // texture vertices
        for (const auto& vt : mesh.texture_vertices) {
            file << "vt " << vt.u << " " << vt.v << '\n';
        }
        file << '\n';

        // normals
        for (const auto& n : mesh.normals) {
            file << "vn " << n.x << " " << n.y << " " << n.z << '\n';
        }
        file << '\n';

        // faces
        for (const auto& vec : mesh.faces) {
            file << "f ";
            for (const auto [vi, ti, ni] : vec) {
                file << vi + 1; // 1
                if (ti != -1) {
                    file << '/' << ti + 1; // 1/2
                    if (ni != -1) {
                        file << '/' << ni + 1; // 1/2/3
                    }
                } else if (ni != -1) {
                    file << "//" << ni + 1; // 1//3
                }

                file << " ";
            }
            file << '\n';
        }
    }
} // namespace di_renderer::io
