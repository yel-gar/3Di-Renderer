#include "OpenGLArea.hpp"

#include "Triangle.hpp"
#include "core/AppData.hpp"
#include "core/RenderMode.hpp"
#include "math/Camera.hpp"

#include <chrono>
#include <cstring>
#include <epoxy/gl.h>
#include <filesystem>
#include <fstream>
#include <gdkmm/pixbuf.h>
#include <glibmm/error.h>
#include <iostream>
#include <limits>
#include <vector>

using di_renderer::render::OpenGLArea;

OpenGLArea::OpenGLArea() {
    set_has_depth_buffer(true);
    set_auto_render(true);
    set_required_version(3, 3);
    set_can_focus(true);

    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

    auto& camera = m_app_data.get_current_camera();
    camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
    camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
    camera.set_planes(0.1f, 100.0f);
    camera.set_fov(45.0f * static_cast<float>(M_PI) / 180.0f);
    camera.set_aspect_ratio(1.0f);

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
    m_app_data.get_current_camera().set_aspect_ratio(aspect_ratio);
}

bool OpenGLArea::on_button_press_event(GdkEventButton* event) {
    if (event->button == 1) {
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

    double dx = event->x - m_last_x;
    double dy = event->y - m_last_y;

    m_last_x = event->x;
    m_last_y = event->y;

    m_app_data.get_current_camera().rotate_view(dx, dy);
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

void OpenGLArea::parse_keyboard_movement() {
    math::Vector3 vec;

    if (key_pressed(GDK_KEY_w) || key_pressed(GDK_KEY_W))
        vec.z += 1.f;
    if (key_pressed(GDK_KEY_s) || key_pressed(GDK_KEY_S))
        vec.z -= 1.f;
    if (key_pressed(GDK_KEY_a) || key_pressed(GDK_KEY_A))
        vec.x -= 1.f;
    if (key_pressed(GDK_KEY_d) || key_pressed(GDK_KEY_D))
        vec.x += 1.f;
    if (key_pressed(GDK_KEY_q) || key_pressed(GDK_KEY_Q))
        vec.y -= 1.f;
    if (key_pressed(GDK_KEY_e) || key_pressed(GDK_KEY_E))
        vec.y += 1.f;

    if (vec.length() <= std::numeric_limits<float>::epsilon()) {
        return;
    }

    m_app_data.get_current_camera().move(vec.normalized());
}

bool OpenGLArea::key_pressed(unsigned int key) {
    return m_pressed_keys.find(key) != m_pressed_keys.end();
}

void OpenGLArea::update_camera_for_mesh() {
    auto& app_data = get_app_data();
    if (!m_gl_initialized.load() || app_data.is_meshes_empty()) {
        return;
    }

    auto& camera = app_data.get_current_camera();
    try {
        const auto& mesh = app_data.get_current_mesh();

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

            camera.set_position(math::Vector3(center.x, center.y, center.z + distance));
            camera.set_target(center);

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

            camera.set_planes(near_plane, far_plane);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading mesh for camera update: " << e.what() << '\n';
        camera.set_position(math::Vector3(0.0f, 0.0f, 3.0f));
        camera.set_target(math::Vector3(0.0f, 0.0f, 0.0f));
        camera.set_planes(0.1f, 100.0f);
    }

    int width = get_width();
    int height = get_height();
    if (width > 0 && height > 0) {
        float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        camera.set_aspect_ratio(aspect_ratio);
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

    auto& camera = get_app_data().get_current_camera();
    try {
        const auto& mesh = get_app_data().get_current_mesh();
        if (mesh.vertices.empty()) {
            return;
        }

        math::Vector3 camera_pos = camera.get_position();
        math::Vector3 target = camera.get_target();
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

        camera.set_planes(near_plane, far_plane);
    } catch (...) {
    }
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

    if (m_shader_program != 0u) {
        di_renderer::graphics::destroy_mesh_batch();
        di_renderer::graphics::destroy_shader_program(m_shader_program);
        m_shader_program = 0;
    }

    for (const auto& pair : m_loaded_textures) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    m_loaded_textures.clear();
}

void OpenGLArea::resolve_texture_path(std::string& texture_path, const std::string& base_path) {
    namespace fs = std::filesystem;

    if (fs::exists(texture_path)) {
        return;
    }

    fs::path base_dir = fs::path(base_path).parent_path();
    fs::path relative_path = base_dir / texture_path;

    if (fs::exists(relative_path)) {
        texture_path = relative_path.string();
        return;
    }

    std::vector<std::string> subdirs = {"textures", "texture", "tex", "images", "img"};
    for (const auto& subdir : subdirs) {
        fs::path subdir_path = base_dir / subdir / fs::path(texture_path).filename();
        if (fs::exists(subdir_path)) {
            texture_path = subdir_path.string();
            return;
        }
    }

    fs::path simple_path = base_dir / fs::path(texture_path).filename();
    if (fs::exists(simple_path)) {
        texture_path = simple_path.string();
    }
}

GLuint OpenGLArea::load_texture_from_file(const std::string& filename, const std::string& base_path) {
    std::string texture_path = filename;
    resolve_texture_path(texture_path, base_path);

    try {
        if (!std::filesystem::exists(texture_path)) {
            texture_path = filename;
            if (!std::filesystem::exists(texture_path)) {
                std::cerr << "Texture file not found: " << texture_path << std::endl;
                return 0;
            }
        }

        auto pixbuf = Gdk::Pixbuf::create_from_file(texture_path);
        int width = pixbuf->get_width();
        int height = pixbuf->get_height();
        int channels = pixbuf->get_n_channels();
        bool has_alpha = pixbuf->get_has_alpha();

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        guchar* pixels = pixbuf->get_pixels();
        int rowstride = pixbuf->get_rowstride();

        if (has_alpha) {
            std::vector<guchar> rgba_buffer(width * height * 4);
            for (int y = 0; y < height; ++y) {
                const guchar* src_row = pixels + y * rowstride;
                guchar* dst_row = rgba_buffer.data() + y * width * 4;
                for (int x = 0; x < width; ++x) {
                    dst_row[x * 4 + 0] = src_row[x * channels + 0];
                    dst_row[x * 4 + 1] = src_row[x * channels + 1];
                    dst_row[x * 4 + 2] = src_row[x * channels + 2];
                    dst_row[x * 4 + 3] = (channels > 3) ? src_row[x * channels + 3] : 255;
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer.data());
        } else {
            std::vector<guchar> rgb_buffer(width * height * 3);
            for (int y = 0; y < height; ++y) {
                const guchar* src_row = pixels + y * rowstride;
                guchar* dst_row = rgb_buffer.data() + y * width * 3;
                for (int x = 0; x < width; ++x) {
                    dst_row[x * 3 + 0] = src_row[x * channels + 0];
                    dst_row[x * 3 + 1] = src_row[x * channels + 1];
                    dst_row[x * 3 + 2] = src_row[x * channels + 2];
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer.data());
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return texture;
    } catch (const Glib::Error& e) {
        std::cerr << "Failed to load texture from '" << texture_path << "': " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Texture loading error: " << e.what() << std::endl;
    }
    return 0;
}

bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>&) {
    if (!m_gl_initialized.load() || !m_should_render.load() || m_shader_program == 0) {
        return false;
    }

    try {
        make_current();
    } catch (const Glib::Error& e) {
        std::cerr << "Error making GL context current during render: " << e.what() << '\n';
        return false;
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

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << std::endl;
    }

    return true;
}

void OpenGLArea::set_default_uniforms() {
    if ((m_shader_program == 0u) || !m_gl_initialized.load()) {
        return;
    }

    glUseProgram(m_shader_program);

    GLfloat model_matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    const auto& camera = m_app_data.get_current_camera();
    math::Matrix4x4 view_matrix = camera.get_view_matrix();
    math::Matrix4x4 proj_matrix = camera.get_projection_matrix();

    GLint model_loc = glGetUniformLocation(m_shader_program, "uModel");
    GLint view_loc = glGetUniformLocation(m_shader_program, "uView");
    GLint proj_loc = glGetUniformLocation(m_shader_program, "uProjection");
    GLint camera_pos_loc = glGetUniformLocation(m_shader_program, "uCameraPos");
    GLint light_pos_loc = glGetUniformLocation(m_shader_program, "uLightPos");
    GLint texture_loc = glGetUniformLocation(m_shader_program, "uTexture");
    GLint light_color_loc = glGetUniformLocation(m_shader_program, "uLightColor");
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

    math::Vector3 camera_pos = camera.get_position();
    if (camera_pos_loc != -1) {
        glUniform3f(camera_pos_loc, camera_pos.x, camera_pos.y, camera_pos.z);
    }

    math::Vector3 camera_forward = (camera.get_target() - camera_pos).normalized();
    math::Vector3 camera_right = camera_forward.cross(math::Vector3(0.0f, 1.0f, 0.0f)).normalized();
    math::Vector3 camera_up = camera_right.cross(camera_forward).normalized();

    math::Vector3 light_offset = camera_forward * 2.0f + camera_up * 0.5f;
    math::Vector3 light_pos = camera_pos + light_offset;

    if (light_pos_loc != -1) {
        glUniform3f(light_pos_loc, light_pos.x, light_pos.y, light_pos.z);
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

    auto& app_data = get_app_data();

    if (app_data.is_meshes_empty()) {
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
            return;
        }

        bool use_textures = app_data.is_render_mode_enabled(core::RenderMode::TEXTURE);
        bool has_texture = false;
        GLuint texture_id = 0;
        const std::string& tex_filename = mesh.get_texture_filename();
        if (use_textures && !tex_filename.empty()) {
            auto it = m_loaded_textures.find(tex_filename);
            if (it == m_loaded_textures.end()) {
                texture_id = load_texture_from_file(tex_filename, m_current_mesh_path);
                m_loaded_textures[tex_filename] = texture_id;
            } else {
                texture_id = it->second;
            }
            has_texture = (texture_id != 0);
        }

        GLint use_texture_loc = glGetUniformLocation(m_shader_program, "uUseTexture");
        if (use_texture_loc != -1) {
            glUniform1i(use_texture_loc, has_texture ? 1 : 0);
        }

        if (has_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id);
        }

        di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(), indices.size(),
                                                 m_shader_program);

        if (has_texture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error drawing mesh: " << e.what() << '\n';
    }
}
