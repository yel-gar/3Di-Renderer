// NOLINTBEGIN
#include "OpenGLArea.hpp"

#include "Triangle.hpp"
#include "core/AppData.hpp"
#include "render/Camera.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <epoxy/gl.h>
#include <glibmm/dispatcher.h>
#include <iostream>
#include <limits>
#include <vector>

namespace di_renderer::render {

    OpenGLArea::OpenGLArea()
        : m_camera(math::Vector3(0.0f, 0.0f, 3.0f), math::Vector3(0.0f, 0.0f, 0.0f),
                   45.0f * static_cast<float>(M_PI) / 180.0f, 1.0f, 0.1f, 1000.0f) {
        set_required_version(3, 3);
        set_has_depth_buffer(true);
        set_auto_render(false);
        set_can_focus(true);

        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                   Gdk::POINTER_MOTION_MASK | Gdk::FOCUS_CHANGE_MASK);

        m_render_dispatcher.connect(sigc::mem_fun(*this, &OpenGLArea::on_dispatch_render));
    }

    OpenGLArea::~OpenGLArea() {
        stop_animation();
        cleanup_resources();
    }

    void OpenGLArea::ensure_keyboard_focus() {
        set_can_focus(true);
        grab_focus();
        m_has_focus = true;
    }

    bool OpenGLArea::on_focus_in_event(GdkEventFocus* focus_event) {
        m_has_focus = true;
        queue_draw();
        return Gtk::GLArea::on_focus_in_event(focus_event);
    }

    bool OpenGLArea::on_focus_out_event(GdkEventFocus* focus_event) {
        m_has_focus = false;
        return Gtk::GLArea::on_focus_out_event(focus_event);
    }

    void OpenGLArea::on_show() {
        Gtk::GLArea::on_show();
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_hide() {
        Gtk::GLArea::on_hide();
    }

    void OpenGLArea::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Error making GL context current: " << e.what() << '\n';
            return;
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0f);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        m_shader_program = di_renderer::graphics::create_shader_program();

        if (m_shader_program == 0) {
            std::cerr << "Failed to create shader program" << '\n';
            return;
        }

        di_renderer::graphics::init_mesh_batch();
        m_gl_initialized.store(true);
    }

    void OpenGLArea::on_unrealize() {
        m_gl_initialized.store(false);
        cleanup_resources();
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

        float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        m_camera.set_aspect_ratio(aspect_ratio);
    }

    void OpenGLArea::on_map() {
        Gtk::GLArea::on_map();
        m_should_render.store(true);

        if (m_gl_initialized.load()) {
            start_animation();
            update_camera_for_mesh();
        }

        ensure_keyboard_focus();
    }

    void OpenGLArea::on_unmap() {
        m_should_render.store(false);
        stop_animation();
        Gtk::GLArea::on_unmap();
    }

    void OpenGLArea::update_camera_for_mesh() {
        if (!m_gl_initialized.load()) {
            return;
        }

        try {
            const auto& mesh = core::AppData::instance().get_current_mesh();

            if (!mesh.vertices.empty()) {
                math::Vector3 min_pos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                                      std::numeric_limits<float>::max());
                math::Vector3 max_pos(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                                      -std::numeric_limits<float>::max());

                for (const auto& vertex : mesh.vertices) {
                    min_pos.x = std::min(min_pos.x, vertex.x);
                    min_pos.y = std::min(min_pos.y, vertex.y);
                    min_pos.z = std::min(min_pos.z, vertex.z);

                    max_pos.x = std::max(max_pos.x, vertex.x);
                    max_pos.y = std::max(max_pos.y, vertex.y);
                    max_pos.z = std::max(max_pos.z, vertex.z);
                }

                math::Vector3 center((min_pos.x + max_pos.x) * 0.5f, (min_pos.y + max_pos.y) * 0.5f,
                                     (min_pos.z + max_pos.z) * 0.5f);

                math::Vector3 size(max_pos.x - min_pos.x, max_pos.y - min_pos.y, max_pos.z - min_pos.z);

                float max_dimension = std::max({size.x, size.y, size.z});
                float model_radius = max_dimension * 0.5f;

                float fov_rad = 45.0f * static_cast<float>(M_PI) / 180.0f;
                float base_distance = model_radius / std::tan(fov_rad / 2.0f);
                float distance = base_distance * 1.5f;
                distance = std::max(0.5f, distance);
                distance = std::min(500.0f, distance);

                m_camera.set_position(math::Vector3(center.x, center.y, center.z + distance));
                m_camera.set_target(center);

                float near_plane = NAN;
                float far_plane = NAN;

                if (max_dimension < 1.0f) {
                    near_plane = std::max(0.01f, distance * 0.01f);
                    far_plane = (distance * 3.0f) + (max_dimension * 2.0f);
                } else if (max_dimension < 10.0f) {
                    near_plane = std::max(0.1f, distance * 0.05f);
                    far_plane = (distance * 4.0f) + (max_dimension * 2.0f);
                } else {
                    near_plane = std::max(0.5f, distance * 0.1f);
                    far_plane = (distance * 5.0f) + (max_dimension * 2.0f);
                }

                if (far_plane - near_plane < 0.1f * max_dimension) {
                    far_plane = near_plane + (0.1f * max_dimension);
                }

                far_plane = std::min(far_plane, 5000.0f);

                m_camera.set_planes(near_plane, far_plane);

                m_movement_speed = std::clamp(model_radius * 0.8f, 0.2f, 50.0f);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading mesh: " << e.what() << '\n';
            m_camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
            m_camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
            m_camera.set_planes(0.1f, 100.0f);
            m_movement_speed = 2.5f;
        }

        int width = get_width();
        int height = get_height();
        if (width > 0 && height > 0) {
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            m_camera.set_aspect_ratio(aspect_ratio);
        }
    }

    void OpenGLArea::reset_camera_for_new_model() {
        update_camera_for_mesh();
        queue_draw();
    }

    void OpenGLArea::update_dynamic_projection() {
        if (!m_gl_initialized.load()) {
            return;
        }

        try {
            const auto& mesh = core::AppData::instance().get_current_mesh();
            if (mesh.vertices.empty()) {
                return;
            }

            math::Vector3 camera_pos = m_camera.get_position();
            math::Vector3 target = m_camera.get_target();
            float distance = (camera_pos - target).length();

            math::Vector3 min_pos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max());
            math::Vector3 max_pos(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(),
                                  -std::numeric_limits<float>::max());

            for (const auto& vertex : mesh.vertices) {
                min_pos.x = std::min(min_pos.x, vertex.x);
                min_pos.y = std::min(min_pos.y, vertex.y);
                min_pos.z = std::min(min_pos.z, vertex.z);

                max_pos.x = std::max(max_pos.x, vertex.x);
                max_pos.y = std::max(max_pos.y, vertex.y);
                max_pos.z = std::max(max_pos.z, vertex.z);
            }

            math::Vector3 size(max_pos.x - min_pos.x, max_pos.y - min_pos.y, max_pos.z - min_pos.z);
            float max_dimension = std::max({size.x, size.y, size.z});

            float near_plane = NAN;
            float far_plane = NAN;

            if (max_dimension < 1.0f) {
                near_plane = std::max(0.005f, distance * 0.005f);
                far_plane = (distance * 2.5f) + (max_dimension * 3.0f);
            } else if (max_dimension < 10.0f) {
                near_plane = std::max(0.05f, distance * 0.03f);
                far_plane = (distance * 3.5f) + (max_dimension * 2.5f);
            } else {
                near_plane = std::max(0.2f, distance * 0.08f);
                far_plane = (distance * 4.5f) + (max_dimension * 2.0f);
            }

            float min_range = std::max(0.01f, max_dimension * 0.05f);
            if (far_plane - near_plane < min_range) {
                far_plane = near_plane + min_range;
            }

            m_camera.set_planes(near_plane, far_plane);
        } catch (...) {
        }
    }

    bool OpenGLArea::on_button_press_event(GdkEventButton* button_event) {
        if (button_event->button == 1) {
            m_dragging = true;
            m_last_x = button_event->x;
            m_last_y = button_event->y;

            if (!m_has_focus) {
                ensure_keyboard_focus();
            }

            return true;
        }

        return Gtk::GLArea::on_button_press_event(button_event);
    }

    bool OpenGLArea::on_button_release_event(GdkEventButton* button_event) {
        if (button_event->button == 1) {
            m_dragging = false;
            return true;
        }
        return Gtk::GLArea::on_button_release_event(button_event);
    }

    bool OpenGLArea::on_motion_notify_event(GdkEventMotion* motion_event) {
        if (!m_dragging || !m_has_focus) {
            return false;
        }

        double dx = motion_event->x - m_last_x;
        double dy = motion_event->y - m_last_y;
        m_last_x = motion_event->x;
        m_last_y = motion_event->y;

        float sensitivity = 0.2f;

        math::Vector3 position = m_camera.get_position();
        math::Vector3 target = m_camera.get_target();
        math::Vector3 to_target = target - position;
        float distance = to_target.length();

        if (distance < 0.1f) {
            distance = 3.0f;
        }

        math::Vector3 direction = position - target;
        float yaw = std::atan2(direction.x, direction.z);
        float pitch = std::asin(direction.y / distance);

        yaw += dx * sensitivity * static_cast<float>(M_PI) / 180.0f;
        pitch -= dy * sensitivity * static_cast<float>(M_PI) / 180.0f;

        pitch = std::clamp(pitch, -1.55f, 1.55f);

        float cos_pitch = std::cos(pitch);
        math::Vector3 new_position;
        new_position.x = target.x + (std::sin(yaw) * cos_pitch * distance);
        new_position.y = target.y + (std::sin(pitch) * distance);
        new_position.z = target.z + (std::cos(yaw) * cos_pitch * distance);

        m_camera.set_position(new_position);

        if (std::isnan(new_position.x) || std::isnan(new_position.y) || std::isnan(new_position.z)) {
            std::cerr << "ERROR: Invalid camera position calculated!" << '\n';
            m_camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
            m_camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
        }

        queue_draw();
        return true;
    }

    bool OpenGLArea::on_key_press_event(GdkEventKey* key_event) {
        m_pressed_keys.insert(key_event->keyval);
        queue_draw();
        return true;
    }

    bool OpenGLArea::on_key_release_event(GdkEventKey* key_event) {
        m_pressed_keys.erase(key_event->keyval);
        return true;
    }

    void OpenGLArea::parse_keyboard_movement() {
        if (!m_has_focus || !m_gl_initialized.load()) {
            return;
        }

        math::Vector3 forward = m_camera.get_target() - m_camera.get_position();
        forward.y = 0.0f;

        if (forward.length() < 0.001f) {
            forward = math::Vector3(0.0f, 0.0f, -1.0f);
        }

        forward = forward.normalized();

        math::Vector3 right = forward.cross(math::Vector3(0.0f, 1.0f, 0.0f));
        if (right.length() < 0.001f) {
            right = math::Vector3(1.0f, 0.0f, 0.0f);
        } else {
            right = right.normalized();
        }

        math::Vector3 up(0.0f, 1.0f, 0.0f);

        math::Vector3 movement(0.0f, 0.0f, 0.0f);
        bool moved = false;

        if (key_pressed(GDK_KEY_w) || key_pressed(GDK_KEY_W)) {
            movement += forward;
            moved = true;
        }
        if (key_pressed(GDK_KEY_s) || key_pressed(GDK_KEY_S)) {
            movement -= forward;
            moved = true;
        }
        if (key_pressed(GDK_KEY_a) || key_pressed(GDK_KEY_A)) {
            movement -= right;
            moved = true;
        }
        if (key_pressed(GDK_KEY_d) || key_pressed(GDK_KEY_D)) {
            movement += right;
            moved = true;
        }
        if (key_pressed(GDK_KEY_space)) {
            movement += up;
            moved = true;
        }
        if (key_pressed(GDK_KEY_Shift_L) || key_pressed(GDK_KEY_Shift_R)) {
            movement -= up;
            moved = true;
        }

        if (!moved || movement.length() < 0.001f) {
            return;
        }

        movement = movement.normalized();

        static auto last_time = std::chrono::high_resolution_clock::now();
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        if (delta_time > 0.1f) {
            delta_time = 0.016f;
        }

        float speed = m_movement_speed * delta_time;

        math::Vector3 displacement = movement * speed;

        math::Vector3 current_pos = m_camera.get_position();
        math::Vector3 current_target = m_camera.get_target();

        m_camera.set_position(current_pos + displacement);
        m_camera.set_target(current_target + displacement);

        update_dynamic_projection();
    }

    bool OpenGLArea::key_pressed(unsigned int key) {
        return m_pressed_keys.find(static_cast<guint>(key)) != m_pressed_keys.end();
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

        m_render_connection =
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &OpenGLArea::on_animation_timeout), 16);
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

        if (m_shader_program != 0u) {
            di_renderer::graphics::destroy_mesh_batch();
            di_renderer::graphics::destroy_shader_program(m_shader_program);
            m_shader_program = 0;
        }
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

        static auto start_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        float current_frame_time = std::chrono::duration<float>(current_time - start_time).count();
        float elapsed = current_frame_time - m_last_frame_time;
        m_last_frame_time = current_frame_time;

        if (elapsed < 1.0f / 60.0f) {
            return true;
        }

        parse_keyboard_movement();

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

        return true;
    }

    void OpenGLArea::set_default_uniforms() {
        if ((m_shader_program == 0u) || !m_gl_initialized.load()) {
            return;
        }

        glUseProgram(m_shader_program);

        GLfloat model_matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        math::Matrix4x4 view_matrix = m_camera.get_view_matrix();
        math::Matrix4x4 proj_matrix = m_camera.get_projection_matrix();

        GLint model_loc = glGetUniformLocation(m_shader_program, "uModel");
        GLint view_loc = glGetUniformLocation(m_shader_program, "uView");
        GLint proj_loc = glGetUniformLocation(m_shader_program, "uProjection");
        GLint camera_pos_loc = glGetUniformLocation(m_shader_program, "uCameraPos");
        GLint light_pos_loc = glGetUniformLocation(m_shader_program, "uLightPos");
        GLint use_texture_loc = glGetUniformLocation(m_shader_program, "uUseTexture");
        GLint texture_loc = glGetUniformLocation(m_shader_program, "uTexture");
        GLint light_color_loc = glGetUniformLocation(m_shader_program, "uLightColor");
        GLint ambient_color_loc = glGetUniformLocation(m_shader_program, "uAmbientColor");
        GLint ambient_strength_loc = glGetUniformLocation(m_shader_program, "uAmbientStrength");
        GLint diffuse_strength_loc = glGetUniformLocation(m_shader_program, "uDiffuseStrength");
        GLint normal_matrix_loc = glGetUniformLocation(m_shader_program, "uNormalMatrix");

        if (model_loc != -1) {
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, model_matrix);
        }
        if (view_loc != -1) {
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, view_matrix.data());
        }
        if (proj_loc != -1) {
            glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj_matrix.data());
        }

        math::Vector3 camera_pos = m_camera.get_position();
        if (camera_pos_loc != -1) {
            glUniform3f(camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
        }

        math::Vector3 camera_forward = (m_camera.get_target() - camera_pos).normalized();
        math::Vector3 camera_right = camera_forward.cross(math::Vector3(0.0f, 1.0f, 0.0f)).normalized();
        math::Vector3 camera_up = camera_right.cross(camera_forward).normalized();

        math::Vector3 light_offset = camera_forward * 2.0f + camera_up * 0.5f;
        math::Vector3 light_pos = camera_pos + light_offset;

        if (light_pos_loc != -1) {
            glUniform3f(light_pos_loc, light_pos.x, light_pos.y, light_pos.z);
        }

        if (ambient_color_loc != -1) {
            glUniform3f(ambient_color_loc, 0.2f, 0.2f, 0.3f);
        }
        if (ambient_strength_loc != -1) {
            glUniform1f(ambient_strength_loc, 0.4f);
        }
        if (diffuse_strength_loc != -1) {
            glUniform1f(diffuse_strength_loc, 0.8f);
        }

        if (use_texture_loc != -1) {
            glUniform1i(use_texture_loc, 0);
        }
        if (texture_loc != -1) {
            glUniform1i(texture_loc, 0);
        }

        if (light_color_loc != -1) {
            glUniform3f(light_color_loc, 1.0f, 1.0f, 1.0f);
        }

        if (normal_matrix_loc != -1) {
            GLfloat identity_normal_matrix[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
            glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, identity_normal_matrix);
        }
    }

    void OpenGLArea::draw_current_mesh() {
        if ((m_shader_program == 0u) || !m_gl_initialized.load()) {
            return;
        }

        auto& app_data = core::AppData::instance();

        if (!app_data.has_current_mesh()) {
            return;
        }

        try {
            const auto& mesh = app_data.get_current_mesh();

            if (mesh.vertices.empty()) {
                return;
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
                return;
            }

            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                di_renderer::graphics::Vertex vertex{};

                vertex.position[0] = mesh.vertices[i].x;
                vertex.position[1] = mesh.vertices[i].y;
                vertex.position[2] = mesh.vertices[i].z;

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
                    vertex.uv[1] = mesh.texture_vertices[i].v;
                } else {
                    vertex.uv[0] = 0.0f;
                    vertex.uv[1] = 0.0f;
                }

                vertex.color[0] = 1.0f;
                vertex.color[1] = 1.0f;
                vertex.color[2] = 1.0f;

                vertices.push_back(vertex);
            }

            if (!vertices.empty()) {
                di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(),
                                                         indices.size(), m_shader_program);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error drawing mesh: " << e.what() << '\n';
        }
    }
} // namespace di_renderer::render
// NOLINTEND
