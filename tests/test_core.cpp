#include "core/AppData.hpp"

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

    instance.enable_render_mode(RenderMode::POLYGONAL);
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::POLYGONAL));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::VERTICES));

    instance.enable_render_mode(RenderMode::TEXTURE);
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::POLYGONAL));
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::VERTICES));

    instance.disable_render_mode(RenderMode::POLYGONAL);
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::POLYGONAL));
    EXPECT_TRUE(instance.is_render_mode_enabled(RenderMode::TEXTURE));
    EXPECT_FALSE(instance.is_render_mode_enabled(RenderMode::VERTICES));
}

TEST(CoreTests, MeshData) {
    auto& instance = AppData::instance();
    instance.clean();

    Mesh mesh0;
    Mesh mesh1{{{1.0F, 2.0F, 3.0F}}, {}, {}, {}};

    instance.add_mesh(std::move(mesh0));
    instance.add_mesh(std::move(mesh1));

    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 0);
    instance.select_mesh(1);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    EXPECT_THROW(instance.select_mesh(2), std::out_of_range);

    instance.remove_mesh(0);
    EXPECT_EQ(instance.get_current_mesh().vertex_count(), 1);

    instance.remove_mesh(0);
    EXPECT_THROW(instance.get_current_mesh(), std::out_of_range);
}
