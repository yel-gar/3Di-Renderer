#include "core/AppData.hpp"
#include "core/FaceVerticeData.hpp"
#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

#include <core/Mesh.hpp>
#include <gtest/gtest.h>

using di_renderer::core::AppData;
using di_renderer::core::Mesh;
using di_renderer::core::RenderMode;

TEST(CoreTests, InstanceAllocation) {
    const auto& instance = AppData::instance();
    const auto& instance2 = AppData::instance();
    EXPECT_EQ(&instance, &instance2);
}

TEST(CoreTests, RenderModeSetting) {
    auto& instance = AppData::instance();
    instance.clean();

    instance.enable_render_mode(RenderMode::TEXTURE);
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::LIGHTING));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::POLYGON));

    instance.enable_render_mode(RenderMode::LIGHTING);
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::LIGHTING));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::POLYGON));

    instance.disable_render_mode(RenderMode::TEXTURE);
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::LIGHTING));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::POLYGON));
}

TEST(CoreTests, MeshData) {
    auto& instance = AppData::instance();
    instance.clean();

    Mesh mesh0;
    Mesh mesh1{{{}}, {}, {}, {}};

    instance.add_mesh(std::move(mesh0));
    instance.add_mesh(std::move(mesh1));

    instance.select_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 0); // first mesh
    instance.select_mesh(1);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1); // second mesh

    EXPECT_THROW(instance.select_mesh(2), std::out_of_range);

    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1); // second mesh

    instance.remove_mesh(0);
    EXPECT_THROW(instance.get_current_mesh(), std::out_of_range); // no meshes
}

TEST(CoreTests, AdvancedMeshRemove) {
    auto& instance = AppData::instance();
    instance.clean();

    // mesh ladder???
    Mesh mesh0;
    Mesh mesh1{{{}}, {}, {}, {}};
    Mesh mesh2{{{}, {}}, {}, {}, {}};
    Mesh mesh3{{{}, {}, {}}, {}, {}, {}};
    Mesh mesh4{{{}, {}, {}, {}}, {}, {}, {}};

    for (auto m : {mesh0, mesh1, mesh2, mesh3, mesh4}) {
        instance.add_mesh(std::move(m));
    }

    instance.select_mesh(0);
    // (m0) m1 m2 m3 m4

    // case 1: current mesh is 0, remove it
    // expected: mesh1 selected
    // (m1) m2 m3 m4
    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    // case 2: current mesh is 2(i=1), remove 3(i=2)
    // expected: 2 is selected
    // m1 (m2) m4
    instance.select_mesh(1);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);
    instance.remove_mesh(2);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);

    // case 3: mesh before selected is removed
    // expected: 2 is selected
    // (m2) m4
    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);

    // case 4: last mesh is removed (we need to add one more mesh)
    // expected: 4 is selected
    // m2 (m4)
    instance.add_mesh(Mesh{{{}}, {}, {}, {}});
    // index auto selects to last
    instance.remove_mesh(2);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 4);
}

TEST(MeshTriangulationTest, TriangleFaceRemainsUnchanged) {
    std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {0, 1}};
    std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{{0, 0, 0}, {1, 1, 1}, {2, 2, 2}}};

    di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 1);
    auto triangle = mesh.triangulated_faces[0];
    EXPECT_EQ(triangle[0], 0);
    EXPECT_EQ(triangle[1], 1);
    EXPECT_EQ(triangle[2], 2);
}

TEST(MeshTriangulationTest, QuadFaceTriangulatesToTwoTriangles) {
    std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};

    di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 2);

    auto triangle1 = mesh.triangulated_faces[0];
    EXPECT_EQ(triangle1[0], 0);
    EXPECT_EQ(triangle1[1], 1);
    EXPECT_EQ(triangle1[2], 2);

    auto triangle2 = mesh.triangulated_faces[1];
    EXPECT_EQ(triangle2[0], 0);
    EXPECT_EQ(triangle2[1], 2);
    EXPECT_EQ(triangle2[2], 3);
}

TEST(MeshTriangulationTest, PentagonFaceTriangulatesToThreeTriangles) {
    std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {2, 1, 0}, {1, 2, 0}, {0, 1, 0}};
    std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {2, 1}, {1, 2}, {0, 1}};
    std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}}};

    di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 3);

    auto triangle1 = mesh.triangulated_faces[0];
    EXPECT_EQ(triangle1[0], 0);
    EXPECT_EQ(triangle1[1], 1);
    EXPECT_EQ(triangle1[2], 2);

    auto triangle2 = mesh.triangulated_faces[1];
    EXPECT_EQ(triangle2[0], 0);
    EXPECT_EQ(triangle2[1], 2);
    EXPECT_EQ(triangle2[2], 3);

    auto triangle3 = mesh.triangulated_faces[2];
    EXPECT_EQ(triangle3[0], 0);
    EXPECT_EQ(triangle3[1], 3);
    EXPECT_EQ(triangle3[2], 4);
}

TEST(MeshTriangulationTest, SingleFaceWithLessThanThreeVerticesIgnored) {
    std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}};
    std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}};
    std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}};

    std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{
        {0, 0, 0}, {1, 1, 1} // Only 2 vertices
    }};

    di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 0);
}

TEST(MeshTriangulationTest, MultipleFacesAllTriangulated) {
    std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {2, 0, 0}};
    std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {2, 0}};
    std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}},           // Triangle
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}} // Quad
    };

    di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 3); // 1 triangle + 2 from quad = 3
}
