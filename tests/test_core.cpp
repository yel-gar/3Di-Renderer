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
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 0);
    instance.select_mesh(1);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    EXPECT_THROW(instance.select_mesh(2), std::out_of_range);

    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    instance.remove_mesh(0);
    EXPECT_THROW(instance.get_current_mesh(), std::out_of_range);
}

TEST(CoreTests, AdvancedMeshRemove) {
    auto& instance = AppData::instance();
    instance.clean();

    const Mesh mesh0;
    const Mesh mesh1{{{}}, {}, {}, {}};
    const Mesh mesh2{{{}, {}}, {}, {}, {}};
    const Mesh mesh3{{{}, {}, {}}, {}, {}, {}};
    const Mesh mesh4{{{}, {}, {}, {}}, {}, {}, {}};

    for (auto m : {mesh0, mesh1, mesh2, mesh3, mesh4}) {
        instance.add_mesh(std::move(m));
    }

    instance.select_mesh(0);

    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    instance.select_mesh(1);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);
    instance.remove_mesh(2);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);

    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 2);

    instance.add_mesh(Mesh{{{}}, {}, {}, {}});
    instance.remove_mesh(2);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 4);
}

TEST(MeshTriangulationTest, TriangleFaceRemainsUnchanged) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{{0, 0, 0}, {1, 1, 1}, {2, 2, 2}}};

    const di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 1);
    auto triangle = mesh.faces[0];
    EXPECT_EQ(triangle[0].vi, 0);
    EXPECT_EQ(triangle[1].vi, 1);
    EXPECT_EQ(triangle[2].vi, 2);
}

TEST(MeshTriangulationTest, QuadFaceTriangulatesToTwoTriangles) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};

    const di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 2);

    auto triangle1 = mesh.faces[0];
    EXPECT_EQ(triangle1[0].vi, 0);
    EXPECT_EQ(triangle1[1].vi, 1);
    EXPECT_EQ(triangle1[2].vi, 2);

    auto triangle2 = mesh.faces[1];
    EXPECT_EQ(triangle2[0].vi, 0);
    EXPECT_EQ(triangle2[1].vi, 2);
    EXPECT_EQ(triangle2[2].vi, 3);
}

TEST(MeshTriangulationTest, PentagonFaceTriangulatesToThreeTriangles) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {2, 1, 0}, {1, 2, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {2, 1}, {1, 2}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}}};

    const di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 3);

    auto triangle1 = mesh.faces[0];
    EXPECT_EQ(triangle1[0].vi, 0);
    EXPECT_EQ(triangle1[1].vi, 1);
    EXPECT_EQ(triangle1[2].vi, 2);

    auto triangle2 = mesh.faces[1];
    EXPECT_EQ(triangle2[0].vi, 0);
    EXPECT_EQ(triangle2[1].vi, 2);
    EXPECT_EQ(triangle2[2].vi, 3);

    auto triangle3 = mesh.faces[2];
    EXPECT_EQ(triangle3[0].vi, 0);
    EXPECT_EQ(triangle3[1].vi, 3);
    EXPECT_EQ(triangle3[2].vi, 4);
}

TEST(MeshTriangulationTest, SingleFaceWithLessThanThreeVerticesIgnored) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{{0, 0, 0}, {1, 1, 1}}};

    const di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 0);
}

TEST(MeshTriangulationTest, MultipleFacesAllTriangulated) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {2, 0, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {2, 0}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}}, {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};

    const di_renderer::core::Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 3);
}

TEST(MeshNormalComputationTest, ProvidedNormalsArePreserved) {
    const std::vector<di_renderer::math::Vector3> vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(1.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(0.0f, 1.0f, 0.0f)};

    const std::vector<di_renderer::math::Vector3> provided_normals = {di_renderer::math::Vector3(0.0f, 0.0f, 1.0f),
                                                                      di_renderer::math::Vector3(0.0f, 0.0f, 1.0f),
                                                                      di_renderer::math::Vector3(0.0f, 0.0f, 1.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    const Mesh mesh(vertices, {}, provided_normals, faces);

    ASSERT_EQ(mesh.normals.size(), 3u);
    EXPECT_NEAR(mesh.normals[0].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[0].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[0].z, 1.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].z, 1.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].z, 1.0f, 1e-5f);
}

TEST(MeshNormalComputationTest, SingleTriangleNormalComputation) {
    const std::vector<di_renderer::math::Vector3> vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(1.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(0.0f, 1.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    const Mesh mesh(vertices, {}, {}, faces);

    ASSERT_EQ(mesh.normals.size(), 3u);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(mesh.normals[i].x, 0.0f, 1e-5f);
        EXPECT_NEAR(mesh.normals[i].y, 0.0f, 1e-5f);
        EXPECT_NEAR(mesh.normals[i].z, 1.0f, 1e-5f);
        EXPECT_NEAR(mesh.normals[i].length(), 1.0f, 1e-5f);
    }
}

TEST(MeshNormalComputationTest, InvalidIndicesHandling) {
    const std::vector<di_renderer::math::Vector3> vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(1.0f, 0.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {99, -1, -1}}};

    const Mesh mesh(vertices, {}, {}, faces);
    EXPECT_EQ(mesh.normals.size(), 2u);
}

TEST(MeshNormalComputationTest, NormalNormalization) {
    const std::vector<di_renderer::math::Vector3> vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(2.0f, 0.0f, 0.0f),
                                                              di_renderer::math::Vector3(0.0f, 2.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    const Mesh mesh(vertices, {}, {}, faces);

    for (const auto& normal : mesh.normals) {
        EXPECT_NEAR(normal.length(), 1.0f, 1e-5f);
    }
}
