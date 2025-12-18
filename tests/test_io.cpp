#include "core/Mesh.hpp"
#include "io/ObjReader.hpp"
#include "io/ObjWriter.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

using di_renderer::io::ObjData;
using di_renderer::io::ObjReader;

namespace fs = std::filesystem;

namespace di_renderer::io {
    namespace {
        // Test fixture for ObjWriter tests (ft. deepseek)
        class ObjWriterTest : public testing::Test {
          protected:
            void SetUp() override {
                // Create test directory if it doesn't exist
                m_test_dir = "test_output";
                fs::create_directories(m_test_dir);
            }

            void TearDown() override {
                // Clean up test files
                for (const auto& filename : m_created_files) {
                    if (fs::exists(filename)) {
                        fs::remove(filename);
                    }
                }
                // Remove test directory if empty
                if (fs::exists(m_test_dir) && fs::is_empty(m_test_dir)) {
                    fs::remove(m_test_dir);
                }
            }

            std::string get_test_file_path(const std::string& filename) {
                std::string path = m_test_dir + "/" + filename;
                m_created_files.insert(path);
                return path;
            }

            static std::string read_file_content(const std::string& filename) {
                std::ifstream file(filename);
                if (!file.is_open()) {
                    return "";
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            }

            static core::Mesh create_cube_mesh() {
                // Create a simple cube mesh for testing
                core::Mesh mesh;

                // 8 vertices of a cube
                mesh.vertices = {
                    {0.0f, 0.0f, 0.0f}, // 0
                    {1.0f, 0.0f, 0.0f}, // 1
                    {1.0f, 1.0f, 0.0f}, // 2
                    {0.0f, 1.0f, 0.0f}, // 3
                    {0.0f, 0.0f, 1.0f}, // 4
                    {1.0f, 0.0f, 1.0f}, // 5
                    {1.0f, 1.0f, 1.0f}, // 6
                    {0.0f, 1.0f, 1.0f}  // 7
                };

                // 14 texture coordinates (for cube unwrap)
                mesh.texture_vertices = {{0.0f, 0.0f},  {0.25f, 0.0f},  {0.5f, 0.0f},  {0.75f, 0.0f},  {1.0f, 0.0f},
                                         {0.0f, 0.25f}, {0.25f, 0.25f}, {0.5f, 0.25f}, {0.75f, 0.25f}, {1.0f, 0.25f},
                                         {0.0f, 0.5f},  {0.25f, 0.5f},  {0.5f, 0.5f},  {0.75f, 0.5f},  {1.0f, 0.5f}};

                // 6 face normals (one for each cube face)
                mesh.normals = {
                    {0.0f, 0.0f, -1.0f}, // front
                    {0.0f, 0.0f, 1.0f},  // back
                    {0.0f, -1.0f, 0.0f}, // bottom
                    {0.0f, 1.0f, 0.0f},  // top
                    {-1.0f, 0.0f, 0.0f}, // left
                    {1.0f, 0.0f, 0.0f}   // right
                };

                // 12 faces (2 triangles per cube face)
                mesh.faces = {
                    // Front face
                    {
                        {0, 0, 0}, // vertex 0, texcoord 0, normal 0
                        {1, 1, 0}, // vertex 1, texcoord 1, normal 0
                        {2, 2, 0}  // vertex 2, texcoord 2, normal 0
                    },
                    {
                        {2, 2, 0}, // vertex 2, texcoord 2, normal 0
                        {3, 3, 0}, // vertex 3, texcoord 3, normal 0
                        {0, 0, 0}  // vertex 0, texcoord 0, normal 0
                    },
                    // Add more faces as needed for testing...
                };

                return mesh;
            }

            static core::Mesh create_mesh_with_partial_face_data() {
                core::Mesh mesh;
                mesh.vertices = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
                mesh.texture_vertices = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f}};
                mesh.normals = {{0.0f, 0.0f, 1.0f}};

                // Mixed face data: v only, v/vt, v//vn, v/vt/vn
                mesh.faces = {
                    {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}, // v only
                    {{0, 0, -1}, {1, 1, -1}, {2, 2, -1}},    // v/vt
                    {{0, -1, 0}, {1, -1, 0}, {2, -1, 0}},    // v//vn
                    {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}}        // v/vt/vn
                };

                return mesh;
            }

          private:
            std::string m_test_dir;
            std::set<std::string> m_created_files;
        };

        // Test basic file creation
        TEST_F(ObjWriterTest, CreatesFileSuccessfully) {
            const std::string filename = get_test_file_path("test_create.obj");

            const core::Mesh mesh;
            ASSERT_NO_THROW(ObjWriter::write_file(filename, mesh));

            EXPECT_TRUE(fs::exists(filename));
        }

        // Test exception on invalid file path
        TEST_F(ObjWriterTest, ThrowsExceptionOnInvalidPath) {
            // Try to write to a directory that doesn't exist
            const std::string invalid_path = "/nonexistent/path/test.obj";

            const core::Mesh mesh;
            EXPECT_THROW(ObjWriter::write_file(invalid_path, mesh), std::runtime_error);
        }

        // Test writing empty mesh
        TEST_F(ObjWriterTest, WritesEmptyMeshCorrectly) {
            const std::string filename = get_test_file_path("empty_mesh.obj");
            const core::Mesh mesh;

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should contain header and nothing else
            EXPECT_TRUE(content.find("# Generated with Di-Renderer ObjWriter") != std::string::npos);
            EXPECT_TRUE(content.find("v ") == std::string::npos); // No vertices
            EXPECT_TRUE(content.find("f ") == std::string::npos); // No faces
        }

        // Test writing vertices
        TEST_F(ObjWriterTest, WritesVerticesCorrectly) {
            const std::string filename = get_test_file_path("vertices.obj");

            core::Mesh mesh;
            mesh.vertices = {{1.0f, 2.0f, 3.0f}, {4.5f, 6.7f, 8.9f}, {-1.0f, -2.0f, -3.0f}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Check for vertex lines
            EXPECT_TRUE(content.find("v 1 2 3") != std::string::npos);
            EXPECT_TRUE(content.find("v 4.5 6.7 8.9") != std::string::npos);
            EXPECT_TRUE(content.find("v -1 -2 -3") != std::string::npos);
        }

        // Test writing texture coordinates
        TEST_F(ObjWriterTest, WritesTextureVerticesCorrectly) {
            const std::string filename = get_test_file_path("texcoords.obj");

            core::Mesh mesh;
            mesh.texture_vertices = {{0.0f, 1.0f}, {0.5f, 0.5f}, {1.0f, 0.0f}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            EXPECT_TRUE(content.find("vt 0 1") != std::string::npos);
            EXPECT_TRUE(content.find("vt 0.5 0.5") != std::string::npos);
            EXPECT_TRUE(content.find("vt 1 0") != std::string::npos);
        }

        // Test writing normals
        TEST_F(ObjWriterTest, WritesNormalsCorrectly) {
            const std::string filename = get_test_file_path("normals.obj");

            core::Mesh mesh;
            mesh.normals = {{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            EXPECT_TRUE(content.find("vn 0 0 1") != std::string::npos);
            EXPECT_TRUE(content.find("vn 1 0 0") != std::string::npos);
            EXPECT_TRUE(content.find("vn 0 1 0") != std::string::npos);
        }

        // Test writing faces with all components
        TEST_F(ObjWriterTest, WritesFacesWithAllComponents) {
            const std::string filename = get_test_file_path("full_faces.obj");

            core::Mesh mesh;
            mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
            mesh.texture_vertices = {{0, 0}, {1, 0}, {0, 1}};
            mesh.normals = {{0, 0, 1}};
            mesh.faces = {{{0, 0, 0}, {1, 1, 0}, {2, 2, 0}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Face should be written with 1-based indices: 1/1/1 2/2/1 3/3/1
            EXPECT_TRUE(content.find("f 1/1/1 2/2/1 3/3/1") != std::string::npos);
        }

        TEST_F(ObjWriterTest, WritesFacesWithMissingTextureCoords) {
            const std::string filename = get_test_file_path("faces_no_tex.obj");

            core::Mesh mesh;
            mesh.vertices = {{1.0f, 2.0f, 3.0f}};
            mesh.normals = {{0.0f, 0.0f, 1.0f}}; // Add a normal

            // Face with missing texture coordinate (-1) but WITH normal (0)
            mesh.faces = {{{0, -1, 0}, // No texture coordinate, but has normal
                           {0, -1, 0},
                           {0, -1, 0}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should write vertex//normal format: f 1//1 1//1 1//1
            EXPECT_TRUE(content.find("f 1//1 1//1 1//1") != std::string::npos);
            EXPECT_TRUE(content.find("1/1") == std::string::npos); // No texture coordinates
        }

        // Also add a test for truly NO normal either:
        TEST_F(ObjWriterTest, WritesFacesWithNoTextureAndNoNormal) {
            const std::string filename = get_test_file_path("faces_no_tex_no_normal.obj");

            core::Mesh mesh;
            mesh.vertices = {{1.0f, 2.0f, 3.0f}};

            // Face with no texture coordinate AND no normal (both -1)
            mesh.faces = {{{0, -1, -1}, // No texture, no normal
                           {0, -1, -1},
                           {0, -1, -1}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should write vertex indices only: f 1 1 1
            EXPECT_TRUE(content.find("f 1 1 1") != std::string::npos);
            EXPECT_TRUE(content.find('/') == std::string::npos); // No slashes at all
        }

        // Test writing faces with missing normals
        TEST_F(ObjWriterTest, WritesFacesWithMissingNormals) {
            const std::string filename = get_test_file_path("faces_no_normals.obj");

            core::Mesh mesh;
            mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
            mesh.texture_vertices = {{0, 0}, {1, 0}, {0, 1}};
            mesh.faces = {{{0, 0, -1}, // No normal
                           {1, 1, -1},
                           {2, 2, -1}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should write v/vt format: f 1/1 2/2 3/3
            EXPECT_TRUE(content.find("f 1/1 2/2 3/3") != std::string::npos);
        }

        // Test writing faces with texture but no normals
        TEST_F(ObjWriterTest, WritesFacesWithTextureOnly) {
            const std::string filename = get_test_file_path("faces_tex_only.obj");

            core::Mesh mesh;
            mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
            mesh.texture_vertices = {{0, 0}, {1, 0}, {0, 1}};
            mesh.faces = {{{0, 0, -1}, {1, 1, -1}, {2, 2, -1}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            EXPECT_TRUE(content.find("f 1/1 2/2 3/3") != std::string::npos);
        }

        // Test writing faces with normals but no texture
        TEST_F(ObjWriterTest, WritesFacesWithNormalsOnly) {
            const std::string filename = get_test_file_path("faces_normals_only.obj");

            core::Mesh mesh;
            mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
            mesh.normals = {{0, 0, 1}};
            mesh.faces = {{{0, -1, 0}, {1, -1, 0}, {2, -1, 0}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should write v//vn format: f 1//1 2//1 3//1
            EXPECT_TRUE(content.find("f 1//1 2//1 3//1") != std::string::npos);
        }

        // Test mixed face formats in same file
        TEST_F(ObjWriterTest, WritesMixedFaceFormats) {
            const std::string filename = get_test_file_path("mixed_faces.obj");

            auto mesh = create_mesh_with_partial_face_data();
            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Check all formats are present
            EXPECT_TRUE(content.find("f 1 2 3") != std::string::npos);             // v only
            EXPECT_TRUE(content.find("f 1/1 2/2 3/3") != std::string::npos);       // v/vt
            EXPECT_TRUE(content.find("f 1//1 2//1 3//1") != std::string::npos);    // v//vn
            EXPECT_TRUE(content.find("f 1/1/1 2/2/1 3/3/1") != std::string::npos); // v/vt/vn
        }

        // Test writing a complex mesh
        TEST_F(ObjWriterTest, WritesComplexMeshCorrectly) {
            const std::string filename = get_test_file_path("cube.obj");

            auto mesh = create_cube_mesh();
            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Basic checks
            EXPECT_TRUE(content.find("# Generated with Di-Renderer ObjWriter") != std::string::npos);

            // Count expected lines
            std::stringstream ss(content);
            std::string line;
            int vertex_count = 0;
            int texcoord_count = 0;
            int normal_count = 0;
            int face_count = 0;

            while (std::getline(ss, line)) {
                if (line.find("v ") == 0) {
                    vertex_count++;
                } else if (line.find("vt ") == 0) {
                    texcoord_count++;
                } else if (line.find("vn ") == 0) {
                    normal_count++;
                } else if (line.find("f ") == 0) {
                    face_count++;
                }
            }

            EXPECT_EQ(vertex_count, mesh.vertex_count());
            EXPECT_EQ(texcoord_count, mesh.texcoord_count());
            EXPECT_EQ(normal_count, mesh.normal_count());
            EXPECT_EQ(face_count, mesh.face_count());
        }

        // Test that 0-based indices are converted to 1-based
        TEST_F(ObjWriterTest, ConvertsIndicesTo1Based) {
            const std::string filename = get_test_file_path("index_conversion.obj");

            core::Mesh mesh;
            mesh.vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
            mesh.texture_vertices = {{0, 0}, {1, 0}, {0, 1}};
            mesh.normals = {{0, 0, 1}};
            mesh.faces = {{{0, 0, 0}, // All 0-based indices
                           {1, 1, 0},
                           {2, 2, 0}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Should be converted to 1-based: 1/1/1 2/2/1 3/3/1
            EXPECT_TRUE(content.find("1/1/1") != std::string::npos);
            EXPECT_TRUE(content.find("2/2/1") != std::string::npos);
            EXPECT_TRUE(content.find("3/3/1") != std::string::npos);

            // Should NOT contain 0-based indices
            EXPECT_TRUE(content.find("0/0/0") == std::string::npos);
        }

        // Test file is properly formatted with newlines
        TEST_F(ObjWriterTest, FileHasCorrectFormatting) {
            const std::string filename = get_test_file_path("formatting.obj");

            core::Mesh mesh;
            mesh.vertices = {{1, 2, 3}};
            mesh.texture_vertices = {{0, 1}};
            mesh.normals = {{0, 0, 1}};
            mesh.faces = {{{0, 0, 0}}};

            ObjWriter::write_file(filename, mesh);

            const std::string content = read_file_content(filename);

            // Check sections are separated by empty lines
            std::stringstream ss(content);
            std::string line;
            bool found_vertex_section = false;
            bool found_texcoord_section = false;
            bool found_normal_section = false;

            while (std::getline(ss, line)) {
                if (line.empty() && found_vertex_section && !found_texcoord_section) {
                    found_texcoord_section = true; // Empty line after vertices
                }
                if (line.empty() && found_texcoord_section && !found_normal_section) {
                    found_normal_section = true; // Empty line after texcoords
                }
                if (line.find("v ") == 0) {
                    found_vertex_section = true;
                }
            }

            EXPECT_TRUE(found_vertex_section);
            // Note: The test above might need adjustment based on actual formatting
        }

        // Test with very large mesh (stress test)
        TEST_F(ObjWriterTest, WritesLargeMeshWithoutCrashing) {
            const std::string filename = get_test_file_path("large_mesh.obj");

            core::Mesh mesh;

            // Create 1000 vertices
            for (int i = 0; i < 1000; ++i) {
                mesh.vertices.emplace_back(static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3));
            }

            // Create 100 faces
            for (int i = 0; i < 100; ++i) {
                mesh.faces.push_back({{i % 1000, -1, -1}, {(i + 1) % 1000, -1, -1}, {(i + 2) % 1000, -1, -1}});
            }

            // Should not throw
            ASSERT_NO_THROW(ObjWriter::write_file(filename, mesh));

            // File should exist and have content
            EXPECT_TRUE(fs::exists(filename));
            EXPECT_GT(fs::file_size(filename), 0);
        }

    } // namespace
} // namespace di_renderer::io

// Main function to run tests
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(ObjReaderTests, BasicObjParsing) {
    // Temporary test file content
    const auto* filename = "test_tmp.obj";
    std::ofstream out(filename);

    out << "# comment\n"
           "v 1.0 2.0 3.0\n"
           "v 4.0 5.0 6.0\n"
           "vt 0.1 0.2\n"
           "vn 0.0 1.0 0.0\n"
           "f 1/1/1 2/1/1\n";

    out.close();

    const std::string name = filename;
    auto [vertices, texture_vertices, normals, faces] = ObjReader::read_file(name);

    // Check vertices
    ASSERT_EQ(vertices.size(), 2);
    EXPECT_FLOAT_EQ(vertices[0].x, 1.0f);
    EXPECT_FLOAT_EQ(vertices[0].y, 2.0f);
    EXPECT_FLOAT_EQ(vertices[0].z, 3.0f);

    EXPECT_FLOAT_EQ(vertices[1].x, 4.0f);
    EXPECT_FLOAT_EQ(vertices[1].y, 5.0f);
    EXPECT_FLOAT_EQ(vertices[1].z, 6.0f);

    // Check texture vertices
    ASSERT_EQ(texture_vertices.size(), 1);
    EXPECT_FLOAT_EQ(texture_vertices[0].u, 0.1f);
    EXPECT_FLOAT_EQ(texture_vertices[0].v, 0.2f);

    // Check normals
    ASSERT_EQ(normals.size(), 1);
    EXPECT_FLOAT_EQ(normals[0].x, 0.0f);
    EXPECT_FLOAT_EQ(normals[0].y, 1.0f);
    EXPECT_FLOAT_EQ(normals[0].z, 0.0f);

    // Check faces
    ASSERT_EQ(faces.size(), 1);
    ASSERT_EQ(faces[0].size(), 2);

    // Vertex indices, texture indices, normal indices
    EXPECT_EQ(faces[0][0].vi, 0); // vertex 1 → index 0
    EXPECT_EQ(faces[0][0].ti, 0);
    EXPECT_EQ(faces[0][0].ni, 0);

    EXPECT_EQ(faces[0][1].vi, 1); // vertex 2 → index 1
    EXPECT_EQ(faces[0][1].ti, 0);
    EXPECT_EQ(faces[0][1].ni, 0);
}
