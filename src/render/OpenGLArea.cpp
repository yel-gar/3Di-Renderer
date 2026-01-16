#include "OpenGLArea.hpp"

#include "Triangle.hpp"
#include "core/AppData.hpp"
#include "core/RenderMode.hpp"
#include "math/Camera.hpp"
#include "math/Matrix4x4.hpp"
#include "math/Transform.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <epoxy/gl.h>
#include <gdkmm/pixbuf.h>
#include <glibmm/error.h>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

using di_renderer::render::OpenGLArea;

OpenGLArea::OpenGLArea()
    : m_scene_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max()),
      m_scene_max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                  -std::numeric_limits<float>::max()) {
    set_has_depth_buffer(true);
    set_auto_render(true);
    set_required_version(3, 3);
    set_can_focus(true);

    // mouse events
    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK);

    // keyboard events
    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

    m_render_dispatcher.connect(sigc::mem_fun(*this, &OpenGLArea::on_dispatch_render));
}

OpenGLArea::~OpenGLArea() {
    stop_animation();
    cleanup_resources();
}

void OpenGLArea::on_realize() {
    Gtk::GLArea::on_realize();

    try {
        make_current();
    } catch (const Glib::Error& e) {
        std::cerr << "Error making GL context current: " << e.what() << '\n';
        return;
    }

    GLint depth_bits = 0;
    glGetIntegerv(GL_DEPTH_BITS, &depth_bits);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_MULTISAMPLE);

    m_shader_program = di_renderer::graphics::create_shader_program();

    if (m_shader_program == 0) {
        std::cerr << "Failed to create shader program" << '\n';
        return;
    }

    di_renderer::graphics::init_mesh_batch();
    m_gl_initialized.store(true);
}

void OpenGLArea::on_unrealize() {
    cleanup_resources();
    m_gl_initialized.store(false);
    Gtk::GLArea::on_unrealize();
}

void OpenGLArea::on_resize(int width, int height) {
    Gtk::GLArea::on_resize(width, height);

    if (!m_gl_initialized.load() || width <= 0 || height <= 0) {
        return;
    }

    try {
        make_current();
    } catch (const Glib::Error& e) {
        return;
    }

    glViewport(0, 0, width, height);

    const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    m_app_data.get_current_camera().set_aspect_ratio(aspect_ratio);
    update_dynamic_projection();
}

bool OpenGLArea::on_button_press_event(GdkEventButton* event) {
    if (event->button == 1) { // Left mouse button
        grab_focus();
        m_dragging = true;
        m_last_x = event->x;
        m_last_y = event->y;
        return true;
    }
    return false;
}

bool OpenGLArea::on_button_release_event(GdkEventButton* event) {
    if (event->button == 1) {
        m_dragging = false;
        return true;
    }
    return false;
}

bool OpenGLArea::on_motion_notify_event(GdkEventMotion* event) {
    if (!m_dragging) {
        return false;
    }

    const double dx = event->x - m_last_x;
    const double dy = event->y - m_last_y;

    m_last_x = event->x;
    m_last_y = event->y;

    m_app_data.get_current_camera().rotate_view(static_cast<float>(dx), static_cast<float>(dy));
    queue_draw();
    return true;
}

bool OpenGLArea::on_key_press_event(GdkEventKey* event) {
    m_pressed_keys.insert(event->keyval);
    parse_keyboard_movement();
    queue_draw();
    return true;
}

bool OpenGLArea::on_key_release_event(GdkEventKey* event) {
    m_pressed_keys.erase(event->keyval);
    parse_keyboard_movement();
    return true;
}
bool OpenGLArea::on_scroll_event(GdkEventScroll* scroll_event) {
    float val = 0;
    switch (scroll_event->direction) {
    case GDK_SCROLL_UP:
        val = 1.0f;
        break;
    case GDK_SCROLL_DOWN:
        val = -1.0f;
        break;
    default:
        break;
    }
    m_app_data.get_current_camera().zoom(val);
    return true;
}

void OpenGLArea::parse_keyboard_movement() {
    di_renderer::math::Vector3 vec(0.0f, 0.0f, 0.0f);

    if (key_pressed(GDK_KEY_w) || key_pressed(GDK_KEY_W)) {
        vec.z += 1.f;
    }
    if (key_pressed(GDK_KEY_s) || key_pressed(GDK_KEY_S)) {
        vec.z -= 1.f;
    }
    if (key_pressed(GDK_KEY_a) || key_pressed(GDK_KEY_A)) {
        vec.x -= 1.f;
    }
    if (key_pressed(GDK_KEY_d) || key_pressed(GDK_KEY_D)) {
        vec.x += 1.f;
    }
    if (key_pressed(GDK_KEY_q) || key_pressed(GDK_KEY_Q)) {
        vec.y -= 1.f;
    }
    if (key_pressed(GDK_KEY_e) || key_pressed(GDK_KEY_E)) {
        vec.y += 1.f;
    }

    if (vec.length() <= std::numeric_limits<float>::epsilon()) {
        return;
    }

    m_app_data.get_current_camera().move(vec.normalized());
}

bool OpenGLArea::key_pressed(unsigned int key) {
    return m_pressed_keys.find(key) != m_pressed_keys.end();
}

void OpenGLArea::calculate_camera_planes(const di_renderer::math::Vector3& min_pos, // NOLINT
                                         const di_renderer::math::Vector3& max_pos, float distance,
                                         di_renderer::math::Camera& camera) {
    const di_renderer::math::Vector3 size(max_pos.x - min_pos.x, max_pos.y - min_pos.y, max_pos.z - min_pos.z);
    const float max_dimension = std::max({size.x, size.y, size.z});

    float near_plane = 0.1f;
    float far_plane = 100.0f;

    if (max_dimension < 1.0f) {
        near_plane = std::max(0.01f, distance * 0.01f);
        far_plane = (distance * 5.0f) + (max_dimension * 10.0f);
    } else if (max_dimension < 10.0f) {
        near_plane = std::max(0.1f, distance * 0.05f);
        far_plane = (distance * 10.0f) + (max_dimension * 10.0f);
    } else if (max_dimension < 100.0f) {
        near_plane = std::max(0.5f, distance * 0.1f);
        far_plane = (distance * 20.0f) + (max_dimension * 10.0f);
    } else {
        near_plane = std::max(1.0f, distance * 0.2f);
        far_plane = (distance * 50.0f) + (max_dimension * 10.0f);
    }

    const float min_far_plane = 1000.0f;
    far_plane = std::max(far_plane, min_far_plane);

    const float max_ratio = 10000.0f;
    if (far_plane / near_plane > max_ratio) {
        near_plane = far_plane / max_ratio;
    }

    camera.set_planes(near_plane, far_plane);
}

di_renderer::math::Vector3 OpenGLArea::transform_vertex(const di_renderer::math::Vector3& vertex, // NOLINT
                                                        const di_renderer::math::Transform& transform) {
    const di_renderer::math::Matrix4x4 transform_matrix = transform.get_matrix();
    const di_renderer::math::Vector4 transformed =
        transform_matrix * di_renderer::math::Vector4(vertex.x, vertex.y, vertex.z, 1.0f);
    return {transformed.x, transformed.y, transformed.z};
}

void OpenGLArea::update_scene_bounds() {
    m_scene_min = di_renderer::math::Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                                             std::numeric_limits<float>::max());
    m_scene_max = di_renderer::math::Vector3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                                             -std::numeric_limits<float>::max());
    bool has_vertices = false;

    const auto& meshes = m_app_data.get_meshes();
    for (const auto& mesh : meshes) {
        if (mesh.vertices.empty()) {
            continue;
        }
        has_vertices = true;

        auto& transform = const_cast<di_renderer::core::Mesh&>(mesh).get_transform(); // NOLINT
        auto [min, max] = get_transformed_bounds(mesh, transform);

        m_scene_min = di_renderer::math::Vector3(std::min(m_scene_min.x, min.x), std::min(m_scene_min.y, min.y),
                                                 std::min(m_scene_min.z, min.z));
        m_scene_max = di_renderer::math::Vector3(std::max(m_scene_max.x, max.x), std::max(m_scene_max.y, max.y),
                                                 std::max(m_scene_max.z, max.z));
    }
    m_bounds_valid = has_vertices;
}

std::pair<di_renderer::math::Vector3, di_renderer::math::Vector3>
OpenGLArea::get_transformed_bounds(const di_renderer::core::Mesh& mesh, const di_renderer::math::Transform& transform) {
    di_renderer::math::Vector3 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                                   std::numeric_limits<float>::max());
    di_renderer::math::Vector3 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                                   -std::numeric_limits<float>::max());

    for (const auto& vertex : mesh.vertices) {
        const di_renderer::math::Vector3 transformed = transform_vertex(vertex, transform);
        min = di_renderer::math::Vector3(std::min(min.x, transformed.x), std::min(min.y, transformed.y),
                                         std::min(min.z, transformed.z));
        max = di_renderer::math::Vector3(std::max(max.x, transformed.x), std::max(max.y, transformed.y),
                                         std::max(max.z, transformed.z));
    }
    return {min, max};
}

void OpenGLArea::update_camera_for_mesh() {
    auto& app_data = get_app_data();
    if (!m_gl_initialized.load() || app_data.is_meshes_empty()) {
        return;
    }

    update_scene_bounds();

    if (!m_bounds_valid) {
        auto& camera = app_data.get_current_camera();
        camera = di_renderer::math::Camera(); // Uses fixed default constructor
        return;
    }

    auto& camera = app_data.get_current_camera();
    const di_renderer::math::Vector3 center((m_scene_min.x + m_scene_max.x) * 0.5f,
                                            (m_scene_min.y + m_scene_max.y) * 0.5f,
                                            (m_scene_min.z + m_scene_max.z) * 0.5f);

    const di_renderer::math::Vector3 size(m_scene_max.x - m_scene_min.x, m_scene_max.y - m_scene_min.y,
                                          m_scene_max.z - m_scene_min.z);
    const float max_dimension = std::max({size.x, size.y, size.z});
    const float model_radius = max_dimension * 0.5f;

    const float fov_rad = 45.0f * static_cast<float>(M_PI) / 180.0f;
    const float base_distance = model_radius / std::tan(fov_rad / 2.0f);

    float distance = base_distance * 1.5f;

    distance = std::max(1.0f, distance);
    distance = std::min(500.0f, distance);

    camera.set_position(di_renderer::math::Vector3(center.x, center.y, center.z + distance));
    camera.set_target(center);

    calculate_camera_planes(m_scene_min, m_scene_max, distance, camera);

    const int width = get_width();
    const int height = get_height();
    if (width > 0 && height > 0) {
        const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        camera.set_aspect_ratio(aspect_ratio);
    }
}

void OpenGLArea::update_dynamic_projection() {
    if (!m_gl_initialized.load()) {
        return;
    }

    auto& app_data = get_app_data();
    if (app_data.is_meshes_empty() || !m_bounds_valid) {
        return;
    }

    auto& camera = app_data.get_current_camera();
    const di_renderer::math::Vector3 camera_pos = camera.get_position();
    const di_renderer::math::Vector3 target = camera.get_target();
    const float distance = (camera_pos - target).length();

    calculate_camera_planes(m_scene_min, m_scene_max, distance, camera);
}

void OpenGLArea::reset_camera_for_new_model() {
    m_bounds_valid = false;
    m_app_data.get_current_camera() = di_renderer::math::Camera();
    update_camera_for_mesh();
    queue_draw();
}

void OpenGLArea::on_map() {
    Gtk::GLArea::on_map();
    m_should_render.store(true);
    grab_focus();

    if (m_gl_initialized.load()) {
        start_animation();
        update_camera_for_mesh();
    }
}

void OpenGLArea::on_unmap() {
    m_should_render.store(false);
    stop_animation();
    Gtk::GLArea::on_unmap();
}

di_renderer::core::AppData& OpenGLArea::get_app_data() noexcept {
    return m_app_data;
}

void OpenGLArea::on_dispatch_render() {
    if (m_should_render.load() && get_realized() && get_mapped()) {
        queue_render();
    }
}

void OpenGLArea::start_animation() {
    if (m_render_connection.connected()) {
        return;
    }

    m_render_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &OpenGLArea::on_animation_timeout), 16);
}

bool OpenGLArea::on_animation_timeout() {
    if (!m_should_render.load() || !m_gl_initialized.load()) {
        return false;
    }

    m_render_dispatcher.emit();
    return m_should_render.load();
}

void OpenGLArea::stop_animation() {
    if (m_render_connection.connected()) {
        m_render_connection.disconnect();
    }
}

void OpenGLArea::cleanup_resources() {
    if (!m_gl_initialized.load()) {
        return;
    }

    try {
        make_current();
    } catch (const Glib::Error& e) {
        return;
    }

    for (auto& [_, tex_id] : m_texture_cache) {
        if (tex_id != 0u) {
            glDeleteTextures(1, &tex_id);
        }
    }
    m_texture_cache.clear();

    if (m_shader_program != 0u) {
        glUseProgram(m_shader_program);
        const GLint use_texture_loc = glGetUniformLocation(m_shader_program, "uUseTexture");
        if (use_texture_loc != -1) {
            glUniform1i(use_texture_loc, 0);
        }
        glUseProgram(0);

        di_renderer::graphics::destroy_mesh_batch();
        di_renderer::graphics::destroy_shader_program(m_shader_program);
        m_shader_program = 0;
    }

    m_texture_loader.cleanup();
}

bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>& /*context*/) {
    if (!m_gl_initialized.load() || !m_should_render.load() || m_shader_program == 0) {
        return false;
    }

    try {
        make_current();
    } catch (const Glib::Error& e) {
        std::cerr << "Error making GL context current during render: " << e.what() << '\n';
        return false;
    }

    const int width = get_width();
    const int height = get_height();
    if (width > 0 && height > 0) {
        const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        auto& camera = m_app_data.get_current_camera();
        camera.set_aspect_ratio(aspect_ratio);
    }

    update_dynamic_projection();

    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    set_default_uniforms();
    draw_current_mesh();

    auto& app_data = get_app_data();
    const bool wireframe_mode = app_data.is_render_mode_enabled(core::RenderMode::POLYGON);

    if (wireframe_mode) {
        draw_wireframe_overlay();
    }

    return true;
}

void OpenGLArea::draw_wireframe_overlay() {
    if ((m_shader_program == 0u) || !m_gl_initialized.load() || m_app_data.is_meshes_empty()) {
        return;
    }

    const auto& meshes = m_app_data.get_meshes();
    for (const auto& mesh : meshes) {
        if (mesh.vertices.empty()) {
            continue;
        }

        std::vector<di_renderer::graphics::Vertex> vertices;
        vertices.reserve(mesh.vertices.size());

        std::vector<unsigned int> indices;
        for (const auto& face : mesh.faces) {
            if (face.size() >= 3) {
                indices.push_back(face[0].vi);
                indices.push_back(face[1].vi);
                indices.push_back(face[2].vi);

                for (size_t i = 3; i < face.size(); ++i) {
                    indices.push_back(face[0].vi);
                    indices.push_back(face[i - 1].vi);
                    indices.push_back(face[i].vi);
                }
            }
        }

        if (indices.empty()) {
            continue;
        }

        auto& transform = const_cast<di_renderer::core::Mesh&>(mesh).get_transform(); // NOLINT
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            di_renderer::graphics::Vertex vertex{};

            const di_renderer::math::Vector3 transformed = transform_vertex(mesh.vertices[i], transform);
            vertex.position[0] = transformed.x;
            vertex.position[1] = transformed.y;
            vertex.position[2] = transformed.z;

            if (i < mesh.normals.size()) {
                vertex.normal[0] = mesh.normals[i].x;
                vertex.normal[1] = mesh.normals[i].y;
                vertex.normal[2] = mesh.normals[i].z;
            } else {
                vertex.normal[0] = 0.0f;
                vertex.normal[1] = 1.0f;
                vertex.normal[2] = 0.0f;
            }

            if (i < mesh.texture_vertices.size()) {
                vertex.uv[0] = mesh.texture_vertices[i].u;
                vertex.uv[1] = m_flip_uv_y ? (1.0f - mesh.texture_vertices[i].v) : mesh.texture_vertices[i].v;
            } else {
                vertex.uv[0] = 0.0f;
                vertex.uv[1] = 0.0f;
            }

            vertex.color[0] = 1.0f;
            vertex.color[1] = 0.5f;
            vertex.color[2] = 0.0f;

            vertices.push_back(vertex);
        }

        if (vertices.empty()) {
            continue;
        }

        glUseProgram(m_shader_program);

        const GLint use_texture_loc = glGetUniformLocation(m_shader_program, "uUseTexture");
        if (use_texture_loc != -1) {
            glUniform1i(use_texture_loc, 0);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);

        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.5f);

        di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(), indices.size(),
                                                 m_shader_program);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    glUseProgram(0);
}

void OpenGLArea::set_default_uniforms() { // NOLINT
    if ((m_shader_program == 0u) || !m_gl_initialized.load()) {
        return;
    }

    glUseProgram(m_shader_program);

    std::array<GLfloat, 16> model_matrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    const auto& camera = m_app_data.get_current_camera();
    const di_renderer::math::Matrix4x4 view_matrix = camera.get_view_matrix();
    const di_renderer::math::Matrix4x4 proj_matrix = camera.get_projection_matrix();

    const GLint model_loc = glGetUniformLocation(m_shader_program, "uModel");
    const GLint view_loc = glGetUniformLocation(m_shader_program, "uView");
    const GLint proj_loc = glGetUniformLocation(m_shader_program, "uProjection");
    const GLint camera_pos_loc = glGetUniformLocation(m_shader_program, "uCameraPos");
    const GLint light_pos1_loc = glGetUniformLocation(m_shader_program, "uLightPos1");
    const GLint light_color1_loc = glGetUniformLocation(m_shader_program, "uLightColor1");
    const GLint light_pos2_loc = glGetUniformLocation(m_shader_program, "uLightPos2");
    const GLint light_color2_loc = glGetUniformLocation(m_shader_program, "uLightColor2");
    const GLint use_light2_loc = glGetUniformLocation(m_shader_program, "uUseLight2");
    const GLint texture_loc = glGetUniformLocation(m_shader_program, "uTexture");
    const GLint normal_matrix_loc = glGetUniformLocation(m_shader_program, "uNormalMatrix");

    if (model_loc != -1) {
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, model_matrix.data());
    }
    if (view_loc != -1) {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, view_matrix.data());
    }
    if (proj_loc != -1) {
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj_matrix.data());
    }

    const di_renderer::math::Vector3 camera_pos = camera.get_position();
    if (camera_pos_loc != -1) {
        glUniform3f(camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
    }

    const di_renderer::math::Vector3 camera_forward = (camera.get_target() - camera_pos).normalized();
    const di_renderer::math::Vector3 camera_right =
        camera_forward.cross(di_renderer::math::Vector3(0.0f, 1.0f, 0.0f)).normalized();
    const di_renderer::math::Vector3 camera_up = camera_right.cross(camera_forward).normalized();

    const float light_distance = std::max(2.0f, (camera_pos - camera.get_target()).length() * 0.5f);
    const di_renderer::math::Vector3 light_offset =
        camera_forward * light_distance + camera_up * (light_distance * 0.3f);
    const di_renderer::math::Vector3 light1_pos = camera_pos + light_offset;

    if (light_pos1_loc != -1) {
        glUniform3f(light_pos1_loc, light1_pos.x, light1_pos.y, light1_pos.z);
    }

    if (light_color1_loc != -1) {
        glUniform3f(light_color1_loc, 1.0f, 1.0f, 1.0f);
    }

    auto& app_data = get_app_data();
    const bool lighting_mode = app_data.is_render_mode_enabled(core::RenderMode::LIGHTING);

    if (!m_bounds_valid) {
        const di_renderer::math::Vector3 default_light2_pos(0.0f, 10.0f, 0.0f);

        if (light_pos2_loc != -1) {
            glUniform3f(light_pos2_loc, default_light2_pos.x, default_light2_pos.y, default_light2_pos.z);
        }
        if (light_color2_loc != -1) {
            glUniform3f(light_color2_loc, 0.25f, 0.2f, 0.1f);
        }

        if (use_light2_loc != -1) {
            glUniform1i(use_light2_loc, lighting_mode ? 1 : 0);
        }
    } else {
        if (use_light2_loc != -1) {
            glUniform1i(use_light2_loc, lighting_mode && m_bounds_valid ? 1 : 0);
        }

        if (lighting_mode && m_bounds_valid) {
            const di_renderer::math::Vector3 center((m_scene_min.x + m_scene_max.x) * 0.5f,
                                                    (m_scene_min.y + m_scene_max.y) * 0.5f,
                                                    (m_scene_min.z + m_scene_max.z) * 0.5f);
            const float scene_height = m_scene_max.y - m_scene_min.y;
            const float light2_height = std::max(scene_height * 10.5f, 2.0f);
            const di_renderer::math::Vector3 light2_pos =
                center + di_renderer::math::Vector3(0.0f, light2_height, 0.0f);

            if (light_pos2_loc != -1) {
                glUniform3f(light_pos2_loc, light2_pos.x, light2_pos.y, light2_pos.z);
            }
            if (light_color2_loc != -1) {
                glUniform3f(light_color2_loc, 1.0f, 0.0f, 0.0f);
            }
        }
    }

    if (texture_loc != -1) {
        glUniform1i(texture_loc, 0);
    }

    std::array<GLfloat, 9> identity_normal_matrix = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    if (normal_matrix_loc != -1) {
        glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, identity_normal_matrix.data());
    }
}
void OpenGLArea::draw_current_mesh() { // NOLINT
    if ((m_shader_program == 0u) || !m_gl_initialized.load()) {
        return;
    }

    auto& app_data = get_app_data();

    if (app_data.is_meshes_empty()) {
        return;
    }

    try {
        const auto& meshes = app_data.get_meshes();

        for (const auto& mesh : meshes) {
            if (mesh.vertices.empty()) {
                continue;
            }

            std::vector<di_renderer::graphics::Vertex> vertices;
            vertices.reserve(mesh.vertices.size());

            std::vector<unsigned int> indices;
            for (const auto& face : mesh.faces) {
                if (face.size() >= 3) {
                    indices.push_back(face[0].vi);
                    indices.push_back(face[1].vi);
                    indices.push_back(face[2].vi);

                    for (size_t i = 3; i < face.size(); ++i) {
                        indices.push_back(face[0].vi);
                        indices.push_back(face[i - 1].vi);
                        indices.push_back(face[i].vi);
                    }
                }
            }

            if (indices.empty()) {
                continue;
            }

            auto& transform = const_cast<di_renderer::core::Mesh&>(mesh).get_transform(); // NOLINT
            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                di_renderer::graphics::Vertex vertex{};

                const di_renderer::math::Vector3 transformed = transform_vertex(mesh.vertices[i], transform);
                vertex.position[0] = transformed.x;
                vertex.position[1] = transformed.y;
                vertex.position[2] = transformed.z;

                if (i < mesh.normals.size()) {
                    vertex.normal[0] = mesh.normals[i].x;
                    vertex.normal[1] = mesh.normals[i].y;
                    vertex.normal[2] = mesh.normals[i].z;
                } else {
                    vertex.normal[0] = 0.0f;
                    vertex.normal[1] = 1.0f;
                    vertex.normal[2] = 0.0f;
                }

                if (i < mesh.texture_vertices.size()) {
                    vertex.uv[0] = mesh.texture_vertices[i].u;
                    vertex.uv[1] = m_flip_uv_y ? (1.0f - mesh.texture_vertices[i].v) : mesh.texture_vertices[i].v;
                } else {
                    vertex.uv[0] = 0.0f;
                    vertex.uv[1] = 0.0f;
                }

                vertex.color[0] = 1.0f;
                vertex.color[1] = 1.0f;
                vertex.color[2] = 1.0f;

                vertices.push_back(vertex);
            }

            if (vertices.empty()) {
                continue;
            }

            const std::string& tex_filename = mesh.get_texture_filename();
            const bool has_texture_filename = !tex_filename.empty();
            const bool render_textures = app_data.is_render_mode_enabled(core::RenderMode::TEXTURE);
            bool has_texture = false;
            GLuint texture_id = 0;

            if (has_texture_filename) {
                auto cache_it = m_texture_cache.find(tex_filename);
                if (cache_it != m_texture_cache.end()) {
                    texture_id = cache_it->second;
                    has_texture = (texture_id != 0);
                } else {
                    texture_id = m_texture_loader.load_texture(tex_filename, m_current_mesh_path);
                    has_texture = (texture_id != 0);
                    m_texture_cache[tex_filename] = texture_id;
                }
            }

            const bool use_textures_in_shader = render_textures && has_texture_filename && has_texture;

            glUseProgram(m_shader_program);

            const GLint use_texture_loc = glGetUniformLocation(m_shader_program, "uUseTexture");
            if (use_texture_loc != -1) {
                glUniform1i(use_texture_loc, use_textures_in_shader ? 1 : 0);
            }

            if (use_textures_in_shader && has_texture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_id);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(), indices.size(),
                                                     m_shader_program);

            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error drawing meshes: " << e.what() << '\n';
    }
}

void OpenGLArea::set_current_mesh_path(const std::string& path) {
    m_current_mesh_path = path;
}

void OpenGLArea::on_camera_changed() {
    if (m_gl_initialized.load() && get_realized() && get_mapped()) {
        update_dynamic_projection();
        queue_draw();
    }
}
