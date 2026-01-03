#include "core/AppData.hpp"
#include "core/FaceVerticeData.hpp"
#include "math/Camera.hpp"
#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"

#include <core/Mesh.hpp>
#include <gtest/gtest.h>

using di_renderer::core::AppData;
using di_renderer::core::Mesh;
using di_renderer::core::RenderMode;

constexpr float EPS = std::numeric_limits<float>::epsilon();

TEST(CoreTests, RenderModeSetting) {
    AppData instance;

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
    AppData instance;

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
    AppData instance;

    // mesh ladder???
    const Mesh mesh0;
    const Mesh mesh1{{{}}, {}, {}, {}};
    const Mesh mesh2{{{}, {}}, {}, {}, {}};
    const Mesh mesh3{{{}, {}, {}}, {}, {}, {}};
    const Mesh mesh4{{{}, {}, {}, {}}, {}, {}, {}};

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

TEST(CoreTests, GetCreatesCameraIfMissing) {
    AppData app;

    // default index assumed to be 0
    const di_renderer::math::Camera& cam = app.get_current_camera();

    // Calling again should return the same object
    const di_renderer::math::Camera& cam2 = app.get_current_camera();

    EXPECT_EQ(&cam, &cam2);
}

TEST(CoreTests, SetChangesReturnedCamera) {
    AppData app;

    app.set_current_camera(1);
    const di_renderer::math::Camera& cam1 = app.get_current_camera();

    app.set_current_camera(2);
    const di_renderer::math::Camera& cam2 = app.get_current_camera();

    EXPECT_NE(&cam1, &cam2);
}

TEST(CoreTests, DeleteRemovesCamera) {
    AppData app;

    app.set_current_camera(1);
    di_renderer::math::Camera& cam = app.get_current_camera();
    cam.set_position({200, 200, 200});

    app.delete_current_camera();

    // Re-fetching should create a NEW camera
    const di_renderer::math::Camera& new_cam = app.get_current_camera();

    EXPECT_NE(new_cam.get_position().x, 200);
}

TEST(CoreTests, DeleteWhenMissingDoesNothing) {
    AppData app;

    app.set_current_camera(42);

    // No camera exists yet
    EXPECT_NO_THROW(app.delete_current_camera());
}

TEST(CoreTests, DeleteDoesNotAffectOtherCameras) {
    AppData app;

    app.set_current_camera(1);
    app.get_current_camera(); // just to create

    app.set_current_camera(2);
    const di_renderer::math::Camera& cam2 = app.get_current_camera();

    app.set_current_camera(1);
    app.delete_current_camera();

    app.set_current_camera(2);
    const di_renderer::math::Camera& cam2_again = app.get_current_camera();

    EXPECT_EQ(&cam2, &cam2_again);
}

TEST(MeshTriangulationTest, TriangleFaceRemainsUnchanged) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{{0, 0, 0}, {1, 1, 1}, {2, 2, 2}}};

    Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 1);
    auto triangle = mesh.triangulated_faces[0];
    EXPECT_EQ(triangle[0], 0);
    EXPECT_EQ(triangle[1], 1);
    EXPECT_EQ(triangle[2], 2);
}

TEST(MeshTriangulationTest, QuadFaceTriangulatesToTwoTriangles) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};

    Mesh mesh(vertices, tex_coords, normals, faces);

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
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {2, 1, 0}, {1, 2, 0}, {0, 1, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {2, 1}, {1, 2}, {0, 1}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}}};

    Mesh mesh(vertices, tex_coords, normals, faces);

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
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {{
        {0, 0, 0}, {1, 1, 1} // Only 2 vertices
    }};

    const Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 0);
}

TEST(MeshTriangulationTest, MultipleFacesAllTriangulated) {
    const std::vector<di_renderer::math::Vector3> vertices = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {2, 0, 0}};
    const std::vector<di_renderer::math::UVCoord> tex_coords = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {2, 0}};
    const std::vector<di_renderer::math::Vector3> normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}},           // Triangle
        {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}} // Quad
    };

    const Mesh mesh(vertices, tex_coords, normals, faces);

    EXPECT_EQ(mesh.face_count(), 3); // 1 triangle + 2 from quad = 3
}

TEST(MeshNormalComputationTest, ProvidedNormalsArePreserved) {
    auto vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f), di_renderer::math::Vector3(1.0f, 0.0f, 0.0f),
                     di_renderer::math::Vector3(0.0f, 1.0f, 0.0f)};

    auto provided_normals = {di_renderer::math::Vector3(0.0f, 0.0f, 1.0f), di_renderer::math::Vector3(0.0f, 0.0f, 1.0f),
                             di_renderer::math::Vector3(0.0f, 0.0f, 1.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    Mesh mesh(vertices, {}, provided_normals, faces);

    ASSERT_EQ(mesh.normals.size(), 3u);
    EXPECT_NEAR(mesh.normals[0].x, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[0].y, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[0].z, 1.0f, EPS);
    EXPECT_NEAR(mesh.normals[1].x, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[1].y, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[1].z, 1.0f, EPS);
    EXPECT_NEAR(mesh.normals[2].x, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[2].y, 0.0f, EPS);
    EXPECT_NEAR(mesh.normals[2].z, 1.0f, EPS);
}

TEST(MeshNormalComputationTest, SingleTriangleNormalComputation) {
    auto vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f), di_renderer::math::Vector3(1.0f, 0.0f, 0.0f),
                     di_renderer::math::Vector3(0.0f, 1.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    Mesh mesh(vertices, {}, {}, faces);

    ASSERT_EQ(mesh.normals.size(), 3u);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(mesh.normals[i].x, 0.0f, EPS);
        EXPECT_NEAR(mesh.normals[i].y, 0.0f, EPS);
        EXPECT_NEAR(mesh.normals[i].z, 1.0f, EPS);
        EXPECT_NEAR(mesh.normals[i].length(), 1.0f, EPS);
    }
}

TEST(MeshNormalComputationTest, InvalidIndicesHandling) {
    auto vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f), di_renderer::math::Vector3(1.0f, 0.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {99, -1, -1}}};

    const Mesh mesh(vertices, {}, {}, faces);
    EXPECT_EQ(mesh.normals.size(), 2u);
}

TEST(MeshNormalComputationTest, NormalNormalization) {
    auto vertices = {di_renderer::math::Vector3(0.0f, 0.0f, 0.0f), di_renderer::math::Vector3(2.0f, 0.0f, 0.0f),
                     di_renderer::math::Vector3(0.0f, 2.0f, 0.0f)};

    const std::vector<std::vector<di_renderer::core::FaceVerticeData>> faces = {
        {{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    const Mesh mesh(vertices, {}, {}, faces);

    for (const auto& normal : mesh.normals) {
        EXPECT_NEAR(normal.length(), 1.0f, EPS);
    }
}
