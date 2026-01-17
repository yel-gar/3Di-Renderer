#pragma once
#include "Mesh.hpp"
#include "RenderMode.hpp"
#include "math/Camera.hpp"

#include <bitset>
#include <unordered_map>
#include <gtkmm.h>

namespace di_renderer::core {
    class AppData final {
      public:
        AppData() = default;

        void clean() noexcept;

        bool is_render_mode_enabled(RenderMode mode) const noexcept;
        void enable_render_mode(RenderMode mode) noexcept;
        void disable_render_mode(RenderMode mode) noexcept;
        void set_render_mode(RenderMode mode, bool value) noexcept;

        Mesh& get_current_mesh();
        void add_mesh(Mesh&& mesh) noexcept;
        void remove_mesh(size_t index);
        void select_mesh(size_t index);
        bool move_right();
        bool move_left();
        void remove_current_mesh();
        const std::vector<Mesh>& get_meshes() const noexcept;
        bool is_meshes_empty() const noexcept;
        bool left_button_sensitive() const noexcept;
        bool right_button_sensitive() const noexcept;
        size_t get_current_mesh_index() const noexcept;

        math::Camera& get_current_camera() noexcept;
        void set_current_camera(unsigned int id) noexcept;
        void delete_current_camera() noexcept;

        std::array<math::Vector3, 3> m_lights_positions;
        std::array<Gdk::RGBA, 3> m_light_colors;

      private:
        size_t m_current_mesh_index = 0;
        unsigned int m_current_camera_index = 0;
        std::vector<Mesh> m_meshes;
        std::unordered_map<unsigned int, math::Camera> m_cameras;

        std::bitset<3> m_render_mode;
    };
} // namespace di_renderer::core
