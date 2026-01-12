#include "OpenGLArea.hpp"

#include "Triangle.hpp"
#include "core/AppData.hpp"
#include "math/MatrixTransforms.hpp"
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
        : camera_(math::Vector3(0.0f, 0.0f, 3.0f), math::Vector3(0.0f, 0.0f, 0.0f),
                  45.0f * static_cast<float>(M_PI) / 180.0f, 1.0f, 0.1f, 1000.0f) {
        set_required_version(3, 3);
        set_has_depth_buffer(true);
        set_auto_render(false);
        set_can_focus(true);

        add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                   Gdk::POINTER_MOTION_MASK | Gdk::FOCUS_CHANGE_MASK);

        std::cout << "OpenGLArea created" << std::endl;
        render_dispatcher_.connect(sigc::mem_fun(*this, &OpenGLArea::on_dispatch_render));
    }

    OpenGLArea::~OpenGLArea() {
        stop_animation();
        cleanup_resources();
    }

    // =============================================================================
    // FOCUS MANAGEMENT
    // =============================================================================

    void OpenGLArea::ensure_keyboard_focus() {
        std::cout << "Ensuring keyboard focus..." << std::endl;
        set_can_focus(true);
        grab_focus();
        has_focus_ = true;
        std::cout << "Focus grabbed successfully" << std::endl;
    }

    bool OpenGLArea::on_focus_in_event(GdkEventFocus* focus_event) {
        has_focus_ = true;
        std::cout << "FOCUS IN EVENT - OpenGLArea now has keyboard focus" << std::endl;
        queue_draw();
        return Gtk::GLArea::on_focus_in_event(focus_event);
    }

    bool OpenGLArea::on_focus_out_event(GdkEventFocus* focus_event) {
        has_focus_ = false;
        std::cout << "FOCUS OUT EVENT - OpenGLArea lost keyboard focus" << std::endl;
        return Gtk::GLArea::on_focus_out_event(focus_event);
    }

    // =============================================================================
    // LIFECYCLE MANAGEMENT
    // =============================================================================

    void OpenGLArea::on_show() {
        Gtk::GLArea::on_show();
        std::cout << "OpenGLArea shown" << std::endl;
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_hide() {
        Gtk::GLArea::on_hide();
        std::cout << "OpenGLArea hidden" << std::endl;
    }

    void OpenGLArea::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Error making GL context current: " << e.what() << std::endl;
            return;
        }

        shader_program_ = di_renderer::graphics::create_shader_program();

        if (shader_program_ == 0) {
            std::cerr << "Failed to create shader program" << std::endl;
            return;
        }

        di_renderer::graphics::init_mesh_batch();
        gl_initialized_.store(true);

        std::cout << "GL context realized" << std::endl;
        update_camera_for_mesh();
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_unrealize() {
        gl_initialized_.store(false);
        cleanup_resources();
        Gtk::GLArea::on_unrealize();
        std::cout << "GL context unrealized" << std::endl;
    }

    void OpenGLArea::on_resize(int width, int height) {
        Gtk::GLArea::on_resize(width, height);

        if (!gl_initialized_.load() || width <= 0 || height <= 0)
            return;

        try {
            make_current();
        } catch (const Glib::Error& e) {
            return;
        }

        glViewport(0, 0, width, height);

        float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        camera_.set_aspect_ratio(aspect_ratio);

        std::cout << "Resized to " << width << "x" << height << " | aspect ratio: " << aspect_ratio << std::endl;
    }

    void OpenGLArea::on_map() {
        Gtk::GLArea::on_map();
        should_render_.store(true);

        if (gl_initialized_.load()) {
            start_animation();
            update_camera_for_mesh();
        }

        std::cout << "OpenGLArea mapped" << std::endl;
        ensure_keyboard_focus();
    }

    void OpenGLArea::on_unmap() {
        should_render_.store(false);
        stop_animation();
        Gtk::GLArea::on_unmap();
        std::cout << "OpenGLArea unmapped" << std::endl;
    }

    // =============================================================================
    // CAMERA MANAGEMENT
    // =============================================================================

    void OpenGLArea::update_camera_for_mesh() {
        if (!gl_initialized_.load())
            return;

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
                float distance = std::max(5.0f, model_radius / std::tan(fov_rad / 2.0f));

                camera_.set_position(math::Vector3(center.x, center.y, center.z - distance));
                camera_.set_target(center);

                float near_plane = std::max(0.1f, model_radius * 0.01f);
                float far_plane = std::max(100.0f, model_radius * 100.0f);
                camera_.set_planes(near_plane, far_plane);

                movement_speed_ = std::max(0.5f, model_radius * 0.3f);

                std::cout << "Mesh loaded: center=(" << center.x << "," << center.y << "," << center.z << "), size=("
                          << size.x << "," << size.y << "," << size.z << "), max_dim=" << max_dimension
                          << ", distance=" << distance << ", near/far=(" << near_plane << "/" << far_plane << ")"
                          << ", speed=" << movement_speed_ << std::endl;
            }
        } catch (const std::out_of_range&) {
            camera_.set_position(math::Vector3(0.0f, 0.0f, -3.0f));
            camera_.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
            camera_.set_planes(0.1f, 1000.0f);
            movement_speed_ = 1.0f;
            std::cout << "Using default camera position for test cube" << std::endl;
        }

        int width = get_width();
        int height = get_height();
        if (width > 0 && height > 0) {
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            camera_.set_aspect_ratio(aspect_ratio);
            std::cout << "Aspect ratio set to: " << aspect_ratio << std::endl;
        }
    }

    // =============================================================================
    // INPUT HANDLING
    // =============================================================================

    bool OpenGLArea::on_button_press_event(GdkEventButton* button_event) {
        std::cout << "Mouse button pressed at (" << button_event->x << ", " << button_event->y << ")" << std::endl;

        if (button_event->button == 1) {
            m_dragging = true;
            m_last_x = button_event->x;
            m_last_y = button_event->y;

            if (!has_focus_) {
                ensure_keyboard_focus();
            }

            std::cout << "Mouse drag started" << std::endl;
            return true;
        }

        return Gtk::GLArea::on_button_press_event(button_event);
    }

    bool OpenGLArea::on_button_release_event(GdkEventButton* button_event) {
        if (button_event->button == 1) {
            m_dragging = false;
            std::cout << "Mouse drag ended" << std::endl;
            return true;
        }
        return Gtk::GLArea::on_button_release_event(button_event);
    }

    bool OpenGLArea::on_motion_notify_event(GdkEventMotion* motion_event) {
        if (!m_dragging || !has_focus_) {
            return false;
        }

        double dx = motion_event->x - m_last_x;
        double dy = motion_event->y - m_last_y;
        m_last_x = motion_event->x;
        m_last_y = motion_event->y;

        float sensitivity = 0.2f;
        float yaw = dx * sensitivity;
        float pitch = dy * sensitivity;

        math::Vector3 position = camera_.get_position();
        math::Vector3 target = camera_.get_target();
        math::Vector3 direction = position - target;

        float distance = direction.length();
        if (distance < 0.1f)
            distance = 3.0f;

        float yaw_angle = std::atan2(direction.x, direction.z);
        float pitch_angle = std::asin(direction.y / distance);

        yaw_angle += yaw * static_cast<float>(M_PI) / 180.0f;
        pitch_angle += pitch * static_cast<float>(M_PI) / 180.0f;

        pitch_angle = std::clamp(pitch_angle, -1.5f, 1.5f);

        float x = distance * std::sin(yaw_angle) * std::cos(pitch_angle);
        float y = distance * std::sin(pitch_angle);
        float z = distance * std::cos(yaw_angle) * std::cos(pitch_angle);

        camera_.set_position(math::Vector3(target.x + x, target.y + y, target.z + z));

        std::cout << "Mouse drag: dx=" << dx << ", dy=" << dy << " | Camera orbit: yaw=" << yaw_angle
                  << ", pitch=" << pitch_angle << std::endl;

        queue_draw();
        return true;
    }

    bool OpenGLArea::on_key_press_event(GdkEventKey* key_event) {
        std::cout << "KEY PRESSED: " << key_event->keyval << " (\"" << static_cast<char>(key_event->keyval) << "\")"
                  << " | has_focus_: " << has_focus_ << std::endl;

        pressed_keys_.insert(key_event->keyval);
        parse_keyboard_movement();
        queue_draw();

        math::Vector3 pos = camera_.get_position();
        std::cout << "Camera position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;

        return true;
    }

    bool OpenGLArea::on_key_release_event(GdkEventKey* key_event) {
        std::cout << "KEY RELEASED: " << key_event->keyval << " | has_focus_: " << has_focus_ << std::endl;
        pressed_keys_.erase(key_event->keyval);
        parse_keyboard_movement();
        return true;
    }

    void OpenGLArea::parse_keyboard_movement() {
        if (!has_focus_ || !gl_initialized_.load()) {
            return;
        }

        math::Vector3 movement(0.0f, 0.0f, 0.0f);
        bool moved = false;

        math::Vector3 camera_forward = camera_.get_target() - camera_.get_position();
        camera_forward.y = 0.0f;
        if (camera_forward.length() > 0.001f) {
            float len = camera_forward.length();
            camera_forward = math::Vector3(camera_forward.x / len, camera_forward.y / len, camera_forward.z / len);
        } else {
            camera_forward = math::Vector3(0.0f, 0.0f, 1.0f);
        }

        math::Vector3 camera_right = math::Vector3(camera_forward.z, // right.x = forward.z
                                                   0.0f,             // right.y = 0
                                                   -camera_forward.x // right.z = -forward.x
        );
        float right_len = camera_right.length();
        if (right_len > 0.001f) {
            camera_right =
                math::Vector3(camera_right.x / right_len, camera_right.y / right_len, camera_right.z / right_len);
        } else {
            camera_right = math::Vector3(1.0f, 0.0f, 0.0f);
        }

        math::Vector3 camera_up(0.0f, 1.0f, 0.0f);

        if (key_pressed(GDK_KEY_w) || key_pressed(GDK_KEY_W)) {
            movement.x += camera_forward.x;
            movement.y += camera_forward.y;
            movement.z += camera_forward.z;
            moved = true;
            std::cout << "Moving forward" << std::endl;
        }
        if (key_pressed(GDK_KEY_s) || key_pressed(GDK_KEY_S)) {
            movement.x -= camera_forward.x;
            movement.y -= camera_forward.y;
            movement.z -= camera_forward.z;
            moved = true;
            std::cout << "Moving backward" << std::endl;
        }
        if (key_pressed(GDK_KEY_a) || key_pressed(GDK_KEY_A)) {
            movement.x -= camera_right.x;
            movement.y -= camera_right.y;
            movement.z -= camera_right.z;
            moved = true;
            std::cout << "Moving left" << std::endl;
        }
        if (key_pressed(GDK_KEY_d) || key_pressed(GDK_KEY_D)) {
            movement.x += camera_right.x;
            movement.y += camera_right.y;
            movement.z += camera_right.z;
            moved = true;
            std::cout << "Moving right" << std::endl;
        }

        if (key_pressed(GDK_KEY_space)) {
            movement.x += camera_up.x;
            movement.y += camera_up.y;
            movement.z += camera_up.z;
            moved = true;
            std::cout << "Moving up" << std::endl;
        }
        if (key_pressed(GDK_KEY_Shift_L) || key_pressed(GDK_KEY_Shift_R)) {
            movement.x -= camera_up.x;
            movement.y -= camera_up.y;
            movement.z -= camera_up.z;
            moved = true;
            std::cout << "Moving down" << std::endl;
        }

        if (!moved || movement.length() < 0.001f) {
            return;
        }

        float movement_len = movement.length();
        movement = math::Vector3(movement.x / movement_len, movement.y / movement_len, movement.z / movement_len);
        float speed = movement_speed_ * 0.016f;

        math::Vector3 displacement(movement.x * speed, movement.y * speed, movement.z * speed);
        camera_.move_position(displacement);

        std::cout << "Camera moved by: (" << displacement.x << ", " << displacement.y << ", " << displacement.z << ")"
                  << " | New position: (" << camera_.get_position().x << ", " << camera_.get_position().y << ", "
                  << camera_.get_position().z << ")" << std::endl;
    }

    bool OpenGLArea::key_pressed(unsigned int key) {
        return pressed_keys_.find(key) != pressed_keys_.end();
    }

    // =============================================================================
    // RENDERING & ANIMATION
    // =============================================================================

    void OpenGLArea::on_dispatch_render() {
        if (should_render_.load() && get_realized() && get_mapped()) {
            queue_render();
        }
    }

    void OpenGLArea::start_animation() {
        if (render_connection_.connected())
            return;

        render_connection_ =
            Glib::signal_timeout().connect(sigc::mem_fun(*this, &OpenGLArea::on_animation_timeout), 16);
    }

    bool OpenGLArea::on_animation_timeout() {
        if (!should_render_.load() || !gl_initialized_.load()) {
            return false;
        }

        render_dispatcher_.emit();
        return should_render_.load();
    }

    void OpenGLArea::stop_animation() {
        if (render_connection_.connected()) {
            render_connection_.disconnect();
        }
    }

    void OpenGLArea::cleanup_resources() {
        if (!gl_initialized_.load())
            return;

        try {
            make_current();
        } catch (const Glib::Error& e) {
            return;
        }

        if (shader_program_) {
            di_renderer::graphics::destroy_mesh_batch();
            di_renderer::graphics::destroy_shader_program(shader_program_);
            shader_program_ = 0;
        }
    }

    bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>&) {
        if (!gl_initialized_.load() || !should_render_.load() || shader_program_ == 0) {
            return false;
        }

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Error making GL context current during render: " << e.what() << std::endl;
            return false;
        }

        static auto start_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        float current_frame_time = std::chrono::duration<float>(current_time - start_time).count();
        float elapsed = current_frame_time - last_frame_time_;
        last_frame_time_ = current_frame_time;

        if (elapsed < 1.0f / 60.0f) {
            return true;
        }

        parse_keyboard_movement();

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        set_default_uniforms();
        draw_current_mesh();

        return true;
    }

    void OpenGLArea::set_default_uniforms() {
        if (!shader_program_ || !gl_initialized_.load()) {
            return;
        }

        glUseProgram(shader_program_);

        GLfloat model_matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        math::Matrix4x4 view_matrix = camera_.get_view_matrix();
        math::Matrix4x4 proj_matrix = camera_.get_projection_matrix();

        GLint model_loc = glGetUniformLocation(shader_program_, "uModel");
        GLint view_loc = glGetUniformLocation(shader_program_, "uView");
        GLint proj_loc = glGetUniformLocation(shader_program_, "uProjection");
        GLint camera_pos_loc = glGetUniformLocation(shader_program_, "uCameraPos");
        GLint light_pos_loc = glGetUniformLocation(shader_program_, "uLightPos");
        GLint use_texture_loc = glGetUniformLocation(shader_program_, "uUseTexture");
        GLint texture_loc = glGetUniformLocation(shader_program_, "uTexture");
        GLint light_color_loc = glGetUniformLocation(shader_program_, "uLightColor");

        if (model_loc != -1)
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, model_matrix);
        if (view_loc != -1)
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, view_matrix.data());
        if (proj_loc != -1)
            glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj_matrix.data());

        math::Vector3 camera_pos = camera_.get_position();
        if (camera_pos_loc != -1)
            glUniform3f(camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
        if (light_pos_loc != -1)
            glUniform3f(light_pos_loc, 2.0f, 2.0f, 2.0f);

        if (use_texture_loc != -1)
            glUniform1i(use_texture_loc, 0);
        if (texture_loc != -1)
            glUniform1i(texture_loc, 0);

        if (light_color_loc != -1)
            glUniform3f(light_color_loc, 1.0f, 1.0f, 1.0f);
    }

    void OpenGLArea::draw_current_mesh() {
        if (!shader_program_ || !gl_initialized_.load()) {
            return;
        }

        try {
            const auto& mesh = core::AppData::instance().get_current_mesh();

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

            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                di_renderer::graphics::Vertex vertex;

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
                vertex.color[2] = (std::sin(mesh.vertices[i].z * 2.0f) + 1.0f) * 0.5f;

                vertices.push_back(vertex);
            }

            if (!indices.empty()) {
                di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(),
                                                         indices.size(), shader_program_);
            }
        } catch (const std::out_of_range&) {
            const di_renderer::graphics::Vertex cube_vertices[8] = {
                {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
                {{0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
                {{0.5f, 0.5f, -0.5f}, {0.5f, 1.0f, 0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
                {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}};

            const unsigned int cube_indices[36] = {0, 1, 2, 2, 3, 0,

                                                   7, 6, 5, 5, 4, 7,

                                                   3, 2, 6, 6, 7, 3,

                                                   0, 4, 5, 5, 1, 0,

                                                   1, 5, 6, 6, 2, 1,

                                                   3, 7, 4, 4, 0, 3};

            di_renderer::graphics::draw_indexed_mesh(cube_vertices, 8, cube_indices, 36, shader_program_);
        }
    }

} // namespace di_renderer::render
