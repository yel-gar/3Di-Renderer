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

    // =============================================================================
    // CONSTRUCTOR & DESTRUCTOR
    // =============================================================================

    OpenGLArea::OpenGLArea()
        : m_camera(math::Vector3(0.0f, 0.0f, 3.0f), math::Vector3(0.0f, 0.0f, 0.0f),
                   45.0f * static_cast<float>(M_PI) / 180.0f, 1.0f, 0.1f, 100000.0f) {
        set_required_version(3, 3);
        set_has_depth_buffer(true);
        set_auto_render(false);
        set_can_focus(true);

        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                   Gdk::POINTER_MOTION_MASK | Gdk::FOCUS_CHANGE_MASK);

        std::cout << "OpenGLArea created" << '\n';
        m_render_dispatcher.connect(sigc::mem_fun(*this, &OpenGLArea::on_dispatch_render));
    }

    OpenGLArea::~OpenGLArea() {
        stop_animation();
        cleanup_resources();
    }

    // =============================================================================
    // FOCUS MANAGEMENT
    // =============================================================================

    void OpenGLArea::ensure_keyboard_focus() {
        std::cout << "Ensuring keyboard focus..." << '\n';
        set_can_focus(true);
        grab_focus();
        m_has_focus = true;
        std::cout << "Focus grabbed successfully" << '\n';
    }

    bool OpenGLArea::on_focus_in_event(GdkEventFocus* focus_event) {
        m_has_focus = true;
        std::cout << "FOCUS IN EVENT - OpenGLArea now has keyboard focus" << '\n';
        queue_draw();
        return Gtk::GLArea::on_focus_in_event(focus_event);
    }

    bool OpenGLArea::on_focus_out_event(GdkEventFocus* focus_event) {
        m_has_focus = false;
        std::cout << "FOCUS OUT EVENT - OpenGLArea lost keyboard focus" << '\n';
        return Gtk::GLArea::on_focus_out_event(focus_event);
    }

    // =============================================================================
    // LIFECYCLE MANAGEMENT
    // =============================================================================

    void OpenGLArea::on_show() {
        Gtk::GLArea::on_show();
        std::cout << "OpenGLArea shown" << '\n';
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_hide() {
        Gtk::GLArea::on_hide();
        std::cout << "OpenGLArea hidden" << '\n';
    }

    void OpenGLArea::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Error making GL context current: " << e.what() << '\n';
            return;
        }

        m_shader_program = di_renderer::graphics::create_shader_program();

        if (m_shader_program == 0) {
            std::cerr << "Failed to create shader program" << '\n';
            return;
        }

        di_renderer::graphics::init_mesh_batch();
        m_gl_initialized.store(true);

        std::cout << "GL context realized" << '\n';
        update_camera_for_mesh();
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_unrealize() {
        m_gl_initialized.store(false);
        cleanup_resources();
        Gtk::GLArea::on_unrealize();
        std::cout << "GL context unrealized" << '\n';
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

        std::cout << "Resized to " << width << "x" << height << " | aspect ratio: " << aspect_ratio << '\n';
    }

    void OpenGLArea::on_map() {
        Gtk::GLArea::on_map();
        m_should_render.store(true);

        if (m_gl_initialized.load()) {
            start_animation();
            update_camera_for_mesh();
        }

        std::cout << "OpenGLArea mapped" << '\n';
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_unmap() {
        m_should_render.store(false);
        stop_animation();
        Gtk::GLArea::on_unmap();
        std::cout << "OpenGLArea unmapped" << '\n';
    }

    // =============================================================================
    // CAMERA MANAGEMENT
    // =============================================================================

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

                // IMPROVED distance calculation - better scaling for all model sizes
                float fov_rad = 45.0f * static_cast<float>(M_PI) / 180.0f;
                float base_distance = model_radius / std::tan(fov_rad / 2.0f);

                // Add padding and ensure minimum distance for small models
                float distance = base_distance * 1.5f;
                distance = std::max(2.0f, distance);    // Minimum distance for tiny models
                distance = std::min(1000.0f, distance); // Maximum distance for huge models

                m_camera.set_position(math::Vector3(center.x, center.y, center.z + distance));
                m_camera.set_target(center);

                // IMPROVED far plane calculation - much more aggressive for large models
                float near_plane = std::max(0.1f, model_radius * 0.01f);
                float far_plane = std::max(10000.0f, model_radius * 2000.0f); // 20x larger than before
                m_camera.set_planes(near_plane, far_plane);

                // IMPROVED movement speed - logarithmic scaling for better control
                if (model_radius < 1.0f) {
                    // Small models - slower speed
                    m_movement_speed = 0.5f + model_radius * 2.0f;
                } else if (model_radius < 10.0f) {
                    // Medium models - moderate speed
                    m_movement_speed = 1.0f + model_radius * 0.8f;
                } else {
                    // Large models - faster speed
                    m_movement_speed = 2.0f + model_radius * 0.3f;
                }

                // Ensure speed stays within reasonable bounds
                m_movement_speed = std::clamp(m_movement_speed, 0.2f, 50.0f);

                std::cout << "Mesh loaded: center=(" << center.x << "," << center.y << "," << center.z << "), size=("
                          << size.x << "," << size.y << "," << size.z << "), max_dim=" << max_dimension
                          << ", distance=" << distance << ", near/far=(" << near_plane << "/" << far_plane << ")"
                          << ", speed=" << m_movement_speed << '\n';
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading mesh: " << e.what() << '\n';
            m_camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
            m_camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
            m_camera.set_planes(0.1f, 10000.0f);
            m_movement_speed = 2.5f;
            std::cout << "Using default camera position due to mesh error" << '\n';
        }

        int width = get_width();
        int height = get_height();
        if (width > 0 && height > 0) {
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            m_camera.set_aspect_ratio(aspect_ratio);
            std::cout << "Aspect ratio set to: " << aspect_ratio << '\n';
        }
    }
    void OpenGLArea::reset_camera_for_new_model() {
        update_camera_for_mesh();
        queue_draw();
    }

    // =============================================================================
    // INPUT HANDLING
    // =============================================================================

    bool OpenGLArea::on_button_press_event(GdkEventButton* button_event) {
        std::cout << "Mouse button pressed at (" << button_event->x << ", " << button_event->y << ")" << '\n';

        if (button_event->button == 1) {
            m_dragging = true;
            m_last_x = button_event->x;
            m_last_y = button_event->y;

            if (!m_has_focus) {
                ensure_keyboard_focus();
            }

            std::cout << "Mouse drag started" << '\n';
            return true;
        }

        return Gtk::GLArea::on_button_press_event(button_event);
    }

    bool OpenGLArea::on_button_release_event(GdkEventButton* button_event) {
        if (button_event->button == 1) {
            m_dragging = false;
            std::cout << "Mouse drag ended" << '\n';
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

        // Get current camera state
        math::Vector3 position = m_camera.get_position();
        math::Vector3 target = m_camera.get_target();
        math::Vector3 to_target = target - position;
        float distance = to_target.length();

        if (distance < 0.1f) {
            distance = 3.0f;
        }

        // Calculate current angles FROM target TO camera
        math::Vector3 direction = position - target; // Camera position relative to target
        float yaw = atan2(direction.x, direction.z);
        float pitch = asin(direction.y / distance);

        // Apply rotation (note: dx affects yaw, dy affects pitch)
        yaw += dx * sensitivity * static_cast<float>(M_PI) / 180.0f;
        pitch -= dy * sensitivity * static_cast<float>(M_PI) / 180.0f;

        // Clamp pitch to prevent flipping over
        pitch = std::clamp(pitch, -1.55f, 1.55f); // About 89 degrees

        // Calculate new camera position RELATIVE TO TARGET
        float cos_pitch = cos(pitch);
        math::Vector3 new_position;
        new_position.x = target.x + sin(yaw) * cos_pitch * distance;
        new_position.y = target.y + sin(pitch) * distance;
        new_position.z = target.z + cos(yaw) * cos_pitch * distance;

        // Set new camera position
        m_camera.set_position(new_position);

        // Debug output to catch invalid positions
        if (std::isnan(new_position.x) || std::isnan(new_position.y) || std::isnan(new_position.z)) {
            std::cerr << "ERROR: Invalid camera position calculated!" << '\n';
            m_camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
            m_camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
        }

        queue_draw();
        return true;
    }

    bool OpenGLArea::on_key_press_event(GdkEventKey* key_event) {
        std::cout << "KEY PRESSED: " << key_event->keyval << " (\"" << static_cast<char>(key_event->keyval) << "\")"
                  << " | has_focus_: " << m_has_focus << '\n';

        m_pressed_keys.insert(static_cast<guint>(key_event->keyval));
        queue_draw();

        math::Vector3 pos = m_camera.get_position();
        std::cout << "Camera position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << '\n';

        return true;
    }

    bool OpenGLArea::on_key_release_event(GdkEventKey* key_event) {
        std::cout << "KEY RELEASED: " << key_event->keyval << " | has_focus_: " << m_has_focus << '\n';
        m_pressed_keys.erase(static_cast<guint>(key_event->keyval));
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
    }

    bool OpenGLArea::key_pressed(unsigned int key) {
        return m_pressed_keys.find(static_cast<guint>(key)) != m_pressed_keys.end();
    }

    // =============================================================================
    // RENDERING & ANIMATION
    // =============================================================================

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

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        set_default_uniforms();
        draw_current_mesh();

        glDisable(GL_POLYGON_OFFSET_FILL);

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

        math::Vector3 light_pos = camera_pos + math::Vector3(2.0f, 2.0f, 2.0f);
        if (light_pos_loc != -1) {
            glUniform3f(light_pos_loc, light_pos.x, light_pos.y, light_pos.z);
        }

        if (ambient_color_loc != -1) {
            glUniform3f(ambient_color_loc, 0.2f, 0.2f, 0.3f);
        }
        if (ambient_strength_loc != -1) {
            glUniform1f(ambient_strength_loc, 0.5f);
        }
        if (diffuse_strength_loc != -1) {
            glUniform1f(diffuse_strength_loc, 0.7f);
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
            math::Vector3 camera_forward = (m_camera.get_target() - m_camera.get_position()).normalized();
            math::Vector3 camera_up(0.0f, 1.0f, 0.0f);
            math::Vector3 camera_right = camera_forward.cross(camera_up).normalized();

            GLfloat normal_matrix[9] = {camera_right.x,   camera_right.y,   camera_right.z,
                                        camera_up.x,      camera_up.y,      camera_up.z,
                                        camera_forward.x, camera_forward.y, camera_forward.z};
            glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE, normal_matrix);
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

                vertex.color[0] = (std::sin(mesh.vertices[i].x * 2.0f) + 1.0f) * 0.5f;
                vertex.color[1] = (std::sin(mesh.vertices[i].y * 2.0f) + 1.0f) * 0.5f;
                vertex.color[2] = (std::sin(mesh.vertices[i].z * 2.0f + 2.0f) + 1.0f) * 0.5f;

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
