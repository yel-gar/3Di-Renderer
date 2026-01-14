#include "AppData.hpp"

#include <iostream>
#include <stdexcept>

using di_renderer::core::AppData;
using di_renderer::core::Mesh;

void AppData::clean() noexcept {
    m_meshes.clear();
    m_cameras.clear();
    m_current_mesh_index = 0;
    m_current_camera_index = 0;
    m_render_mode.reset();
}

bool AppData::is_render_mode_enabled(RenderMode mode) const noexcept {
    return m_render_mode.test(static_cast<size_t>(mode));
}

void AppData::enable_render_mode(const RenderMode mode) noexcept {
    set_render_mode(mode, true);
}

void AppData::disable_render_mode(const RenderMode mode) noexcept {
    set_render_mode(mode, false);
}

void AppData::set_render_mode(RenderMode mode, const bool value) noexcept {
    m_render_mode.set(static_cast<size_t>(mode), value);
}

Mesh& AppData::get_current_mesh() {
    if (m_meshes.empty()) {
        throw std::out_of_range("There's no active mesh");
    }
    if (m_current_mesh_index >= m_meshes.size()) {
        std::cerr << "Unexpected behaviour: mesh index (" << m_current_mesh_index << ") was out of bounds ("
                  << m_meshes.size() << ")\n";
        m_current_mesh_index = m_meshes.size() - 1;
    }
    return m_meshes[m_current_mesh_index]; // NOLINT(*-pro-bounds-avoid-unchecked-container-access) because it's checked
}

di_renderer::math::Camera& AppData::get_current_camera() noexcept {
    auto [it, inserted] = m_cameras.try_emplace(m_current_camera_index);
    return it->second;
}

void AppData::set_current_camera(const unsigned int id) noexcept {
    m_current_camera_index = id;
}

void AppData::delete_current_camera() noexcept {
    m_cameras.erase(m_current_camera_index);
    // the index will be updated automatically later by the UI handler
}

void AppData::add_mesh(Mesh&& mesh) noexcept {
    m_meshes.push_back(std::move(mesh));
    m_current_mesh_index = m_meshes.size() - 1;
}

void AppData::remove_mesh(const size_t index) {
    if (index >= m_meshes.size()) {
        throw std::out_of_range("Mesh index out of range");
    }

    m_meshes.erase(m_meshes.begin() + static_cast<std::ptrdiff_t>(index));

    // case: there were 1 or 2 meshes
    if (m_meshes.size() <= 1) {
        m_current_mesh_index = 0;
    }
    // case: selected mesh was the last and got removed
    else if (m_current_mesh_index >= m_meshes.size()) {
        m_current_mesh_index = m_meshes.size() - 1;
    }
    // case: mesh before selected was removed, so we must move current index to the left
    else if (index < m_current_mesh_index) {
        m_current_mesh_index--;
    }

    // case: selected mesh is deleted but is not the last
    // automatically select next mesh due to vector shift

    // case: deleted mesh is after selected one
    // selected mesh stays as it is
}

void AppData::select_mesh(const size_t index) {
    if (index >= m_meshes.size()) {
        throw std::out_of_range("Mesh index out of range");
    }
    m_current_mesh_index = index;
}

void AppData::remove_current_mesh() {
    remove_mesh(m_current_mesh_index);
}

const std::vector<Mesh>& AppData::get_meshes() const noexcept {
    return m_meshes;
}

bool AppData::is_meshes_empty() const noexcept {
    return m_meshes.empty();
}

bool AppData::left_button_sensitive() const noexcept {
    return m_current_mesh_index > 0;
}

bool AppData::right_button_sensitive() const noexcept {
    return m_current_mesh_index + 1 < m_meshes.size();
}

size_t AppData::get_current_mesh_index() const noexcept {
    return m_current_mesh_index;
}

bool AppData::move_right() {
    if (m_current_mesh_index + 1 >= m_meshes.size()) {
        throw std::out_of_range("Mesh index out of range while attempting to move right");
    }
    select_mesh(m_current_mesh_index + 1);
    return m_current_mesh_index + 1 == m_meshes.size();
}

bool AppData::move_left() {
    if (m_current_mesh_index <= 0) {
        throw std::out_of_range("Mesh index out of range while attempting to move left");
    }
    select_mesh(m_current_mesh_index - 1);
    return m_current_mesh_index == 0;
}
