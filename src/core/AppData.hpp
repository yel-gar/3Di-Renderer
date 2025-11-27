#pragma once
#include <bitset>
#include <cstdint>
#include <memory>

#include "Mesh.hpp"
#include "RenderMode.hpp"


namespace di_renderer::core {
    class AppData final {
    public:
        AppData() = default;

        static AppData& instance() noexcept;

        void clean();

        bool is_render_mode_enabled(RenderMode mode) const noexcept;
        void enable_render_mode(RenderMode mode);
        void disable_render_mode(RenderMode mode);

        Mesh& get_current_mesh();
        void add_mesh(Mesh&& mesh) noexcept;
        void remove_mesh(size_t index);
        void select_mesh(size_t index);
    private:
        size_t m_current_mesh_index = 0;
        std::vector<Mesh> m_meshes;

        std::bitset<3> m_render_mode;
    };
}
