#include "AppData.hpp"

#include <stdexcept>

using di_renderer::core::AppData;
using di_renderer::core::Mesh;


AppData& AppData::instance() noexcept {
    static AppData instance;
    return instance;
}

void AppData::clean() {
    m_meshes.clear();
    m_current_mesh_index = 0;
    m_render_mode.reset();
}

bool AppData::is_render_mode_enabled(RenderMode mode) const noexcept {
    return m_render_mode.test(static_cast<size_t>(mode));
}

void AppData::enable_render_mode(RenderMode mode) {
    m_render_mode.set(static_cast<size_t>(mode));
}

void AppData::disable_render_mode(RenderMode mode) {
    m_render_mode.reset(static_cast<size_t>(mode));
}

Mesh & AppData::get_current_mesh() {
    if (m_meshes.empty()) {
        throw std::out_of_range("Mesh list empty");
    }
    return m_meshes[m_current_mesh_index];
}


void AppData::add_mesh(Mesh &&mesh) noexcept {
    m_meshes.push_back(std::move(mesh));
}

void AppData::remove_mesh(const size_t index) {
    if (index >= m_meshes.size()) {
        throw std::out_of_range("Mesh index out of range");
    }

    m_meshes.erase(m_meshes.begin() + static_cast<std::ptrdiff_t>(index));

    if (m_meshes.empty()) {
        m_current_mesh_index = 0;
    } else if (m_current_mesh_index >= m_meshes.size()) {
        m_current_mesh_index = m_meshes.size() - 1;
    }
}

void AppData::select_mesh(const size_t index) {
    if (index >= m_meshes.size()) {
        throw std::out_of_range("Mesh index out of range");
    }
    m_current_mesh_index = index;
}
