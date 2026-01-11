#include "OpenGLArea.hpp"

#include "Triangle.hpp"
#include "core/AppData.hpp"

#include <chrono>
#include <cmath>
#include <epoxy/gl.h>
#include <glibmm/dispatcher.h>
#include <stdexcept>
#include <vector>

namespace di_renderer {
    namespace render {

        OpenGLArea::OpenGLArea() {
            set_required_version(3, 3);
            set_has_depth_buffer(true);
            set_auto_render(false);

            render_dispatcher_.connect(sigc::mem_fun(*this, &OpenGLArea::on_dispatch_render));
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
                return;
            }

            shader_program_ = di_renderer::graphics::create_shader_program();

            if (shader_program_ == 0) {
                return;
            }

            di_renderer::graphics::init_mesh_batch();
            gl_initialized_.store(true);
        }

        void OpenGLArea::on_unrealize() {
            gl_initialized_.store(false);
            cleanup_resources();
            Gtk::GLArea::on_unrealize();
        }

        void OpenGLArea::on_resize(int width, int height) {
            Gtk::GLArea::on_resize(width, height);

            if (!gl_initialized_.load())
                return;

            try {
                make_current();
            } catch (const Glib::Error& e) {
                return;
            }

            if (width > 0 && height > 0) {
                glViewport(0, 0, width, height);
            }
        }

        void OpenGLArea::on_map() {
            Gtk::GLArea::on_map();
            should_render_.store(true);

            if (gl_initialized_.load()) {
                start_animation();
            }
        }

        void OpenGLArea::on_unmap() {
            should_render_.store(false);
            stop_animation();
            Gtk::GLArea::on_unmap();
        }

        bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>& /*context*/) {
            if (!gl_initialized_.load() || !should_render_.load() || shader_program_ == 0) {
                return false;
            }

            try {
                make_current();
            } catch (const Glib::Error& e) {
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

            rotation_angle_ += elapsed * 45.0f;
            if (rotation_angle_ >= 360.0f) {
                rotation_angle_ -= 360.0f;
            }

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

        void OpenGLArea::set_default_uniforms() {
            if (!shader_program_ || !gl_initialized_.load()) {
                return;
            }

            glUseProgram(shader_program_);

            float angle_rad = rotation_angle_ * static_cast<float>(M_PI) / 180.0f;
            GLfloat model_matrix[16] = {cosf(angle_rad), 0.0f, -sinf(angle_rad), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                        sinf(angle_rad), 0.0f, cosf(angle_rad),  0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

            GLfloat view_matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f,
                                       0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -3.0f, 1.0f};

            int width = get_width();
            int height = get_height();
            float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
            if (aspect == 0.0f)
                aspect = 1.0f;

            float fov = 45.0f * static_cast<float>(M_PI) / 180.0f;
            float f = 1.0f / tanf(fov / 2.0f);
            float near_plane = 0.1f;
            float far_plane = 100.0f;

            GLfloat projection_matrix[16] = {f / aspect,
                                             0.0f,
                                             0.0f,
                                             0.0f,
                                             0.0f,
                                             f,
                                             0.0f,
                                             0.0f,
                                             0.0f,
                                             0.0f,
                                             -(far_plane + near_plane) / (far_plane - near_plane),
                                             -1.0f,
                                             0.0f,
                                             0.0f,
                                             -(2.0f * far_plane * near_plane) / (far_plane - near_plane),
                                             0.0f};

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
                glUniformMatrix4fv(view_loc, 1, GL_FALSE, view_matrix);
            if (proj_loc != -1)
                glUniformMatrix4fv(proj_loc, 1, GL_FALSE, projection_matrix);

            if (camera_pos_loc != -1)
                glUniform3f(camera_pos_loc, 0.0f, 0.0f, 3.0f);
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

                    vertex.position = {{mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z}};

                    if (i < mesh.normals.size()) {
                        vertex.normal = {{mesh.normals[i].x, mesh.normals[i].y, mesh.normals[i].z}};
                    } else {
                        vertex.normal = {{0.0f, 1.0f, 0.0f}};
                    }

                    if (i < mesh.texture_vertices.size()) {
                        vertex.uv = {{mesh.texture_vertices[i].u, mesh.texture_vertices[i].v}};
                    } else {
                        vertex.uv = {{0.0f, 0.0f}};
                    }

                    vertex.color = {{(std::sin(mesh.vertices[i].x * 2.0f) + 1.0f) * 0.5f,
                                     (std::sin(mesh.vertices[i].y * 2.0f) + 1.0f) * 0.5f,
                                     (std::sin(mesh.vertices[i].z * 2.0f) + 1.0f) * 0.5f}};

                    vertices.push_back(vertex);
                }

                if (!indices.empty()) {
                    di_renderer::graphics::draw_indexed_mesh(vertices.data(), vertices.size(), indices.data(),
                                                             indices.size(), shader_program_);
                }
            } catch (const std::out_of_range&) {
                const di_renderer::graphics::Vertex cube_vertices[8] = {
                    {{{-0.5f, -0.5f, 0.5f}}, {{1.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f}}}, // 0 - Red
                    {{{0.5f, -0.5f, 0.5f}}, {{0.0f, 1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}, {{1.0f, 0.0f}}},  // 1 - Green
                    {{{0.5f, 0.5f, 0.5f}}, {{0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f}}, {{1.0f, 1.0f}}},   // 2 - Blue
                    {{{-0.5f, 0.5f, 0.5f}}, {{1.0f, 1.0f, 0.0f}}, {{0.0f, 0.0f, 1.0f}}, {{0.0f, 1.0f}}},  // 3 - Yellow
                    {{{-0.5f, -0.5f, -0.5f}},
                     {{1.0f, 0.5f, 0.0f}},
                     {{0.0f, 0.0f, -1.0f}},
                     {{0.0f, 0.0f}}},                                                                      // 4 - Orange
                    {{{0.5f, -0.5f, -0.5f}}, {{0.5f, 0.0f, 1.0f}}, {{0.0f, 0.0f, -1.0f}}, {{1.0f, 0.0f}}}, // 5 - Purple
                    {{{0.5f, 0.5f, -0.5f}}, {{0.5f, 1.0f, 0.5f}}, {{0.0f, 0.0f, -1.0f}}, {{1.0f, 1.0f}}},  // 6 - Pink
                    {{{-0.5f, 0.5f, -0.5f}}, {{0.0f, 0.5f, 0.5f}}, {{0.0f, 0.0f, -1.0f}}, {{0.0f, 1.0f}}}  // 7 - Cyan
                };

                const unsigned int cube_indices[36] = {// Front face
                                                       0, 1, 2, 2, 3, 0,
                                                       // Back face
                                                       7, 6, 5, 5, 4, 7,
                                                       // Top face
                                                       3, 2, 6, 6, 7, 3,
                                                       // Bottom face
                                                       0, 4, 5, 5, 1, 0,
                                                       // Right face
                                                       1, 5, 6, 6, 2, 1,
                                                       // Left face
                                                       3, 7, 4, 4, 0, 3};

                di_renderer::graphics::draw_indexed_mesh(cube_vertices, 8, cube_indices, 36, shader_program_);
            }
        }
    } // namespace render
} // namespace di_renderer
