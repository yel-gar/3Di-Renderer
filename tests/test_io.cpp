#include "io/ObjReader.hpp"

#include <fstream>
#include <gtest/gtest.h>
#include <string>

using di_renderer::io::ObjData;
using di_renderer::io::ObjReader;

TEST(ObjReaderTests, BasicObjParsing)
{
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

    std::string name = filename;
    auto [vertices, texture_vertices, normals, faces] = ObjReader::read_file(name);

    // Check vertices
    ASSERT_EQ(vertices.size(), 2);
    EXPECT_FLOAT_EQ(vertices[0].x, 1.0F);
    EXPECT_FLOAT_EQ(vertices[0].y, 2.0F);
    EXPECT_FLOAT_EQ(vertices[0].z, 3.0F);

    EXPECT_FLOAT_EQ(vertices[1].x, 4.0F);
    EXPECT_FLOAT_EQ(vertices[1].y, 5.0F);
    EXPECT_FLOAT_EQ(vertices[1].z, 6.0F);

    // Check texture vertices
    ASSERT_EQ(texture_vertices.size(), 1);
    EXPECT_FLOAT_EQ(texture_vertices[0].u, 0.1F);
    EXPECT_FLOAT_EQ(texture_vertices[0].v, 0.2F);

    // Check normals
    ASSERT_EQ(normals.size(), 1);
    EXPECT_FLOAT_EQ(normals[0].x, 0.0F);
    EXPECT_FLOAT_EQ(normals[0].y, 1.0F);
    EXPECT_FLOAT_EQ(normals[0].z, 0.0F);

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
