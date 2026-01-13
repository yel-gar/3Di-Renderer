#pragma once
#include "Mesh.hpp"
#include "RenderMode.hpp"

#include <bitset>

namespace di_renderer::core {
    class AppData final {
      public:
        AppData() = default;

        static AppData& instance() noexcept;

        void clean() noexcept;

        bool is_render_mode_enabled(RenderMode mode) const noexcept;
        void enable_render_mode(RenderMode mode) noexcept;
        void disable_render_mode(RenderMode mode) noexcept;
        void set_render_mode(RenderMode mode, bool value) noexcept;

        Mesh& get_current_mesh();
        void add_mesh(Mesh&& mesh) noexcept;
        void remove_mesh(size_t index);
        void select_mesh(size_t index);
        bool has_current_mesh() const noexcept;

      private:
        size_t m_current_mesh_index = 0;
        std::vector<Mesh> m_meshes;

        std::bitset<3> m_render_mode;
    };
} // namespace di_renderer::core
