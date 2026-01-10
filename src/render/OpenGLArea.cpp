#include "OpenGLArea.hpp"

#include <array>
#include <epoxy/gl.h>
#include <iostream>
#include <stdexcept>

namespace di_renderer::render {

    OpenGLArea::OpenGLArea() {
        set_required_version(3, 3);
        set_has_depth_buffer(true);
        set_auto_render(true);
    }

    OpenGLArea::~OpenGLArea() {
        if (shader_program_) {
            cleanup_resources();
        }
    }

    void OpenGLArea::on_realize() {
        Gtk::GLArea::on_realize();

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Failed to make OpenGL context current: " << e.what() << std::endl;
            return;
        }

        init_resources();
    }

    void OpenGLArea::on_unrealize() {
        cleanup_resources();
        Gtk::GLArea::on_unrealize();
    }

    void OpenGLArea::on_resize(int width, int height) {
        Gtk::GLArea::on_resize(width, height);

        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Resize context error: " << e.what() << std::endl;
            return;
        }

        glViewport(0, 0, width, height);
        check_gl_errors("glViewport");
    }

    bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>& context) {
        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Render context error: " << e.what() << std::endl;
            return false;
        }

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        check_gl_errors("Initial setup");

        if (shader_program_) {
            set_default_uniforms();
            draw_test_triangle();
        }

        return true;
    }

    void OpenGLArea::check_gl_errors(const char* operation) const {
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            std::string error_str;
            switch (error) {
            case GL_INVALID_ENUM:
                error_str = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error_str = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error_str = "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error_str = "GL_OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error_str = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                error_str = "Unknown error";
            }
            std::cerr << "OpenGL error during " << operation << ": " << error_str << " (0x" << std::hex << error << ")"
                      << std::dec << std::endl;
        }
    }

    void OpenGLArea::init_resources() {
        shader_program_ = di_renderer::graphics::create_shader_program();
        if (!shader_program_) {
            std::cerr << "Failed to create shader program" << std::endl;
            return;
        }

        di_renderer::graphics::init_triangle_batch();
        check_gl_errors("init_resources");
    }

    void OpenGLArea::cleanup_resources() {
        try {
            make_current();
        } catch (const Glib::Error& e) {
            std::cerr << "Cleanup context error: " << e.what() << std::endl;
            return;
        }

        if (shader_program_) {
            di_renderer::graphics::destroy_triangle_batch();
            di_renderer::graphics::destroy_shader_program(shader_program_);
            shader_program_ = 0;
        }
    }

    void OpenGLArea::set_default_uniforms() {
        glUseProgram(shader_program_);

        GLfloat identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "uModel"), 1, GL_FALSE, identity);
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "uView"), 1, GL_FALSE, identity);
        glUniformMatrix4fv(glGetUniformLocation(shader_program_, "uProjection"), 1, GL_FALSE, identity);

        glUniform3f(glGetUniformLocation(shader_program_, "uCameraPos"), 0.0f, 0.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shader_program_, "uLightPos"), 0.0f, 0.0f, 5.0f);
        glUniform1i(glGetUniformLocation(shader_program_, "uUseTexture"), 0);

        check_gl_errors("set_default_uniforms");
    }
    void OpenGLArea::draw_test_triangle() {
        const std::array<di_renderer::graphics::Vertex, 3> triangle_vertices = {
            {{
                 std::array<float, 3>{-0.5f, -0.5f, 0.0f}, // position
                 std::array<float, 3>{1.0f, 0.0f, 0.0f},   // color
                 std::array<float, 3>{0.0f, 0.0f, 1.0f},   // normal
                 std::array<float, 2>{0.0f, 0.0f}          // uv
             },
             {
                 std::array<float, 3>{0.5f, -0.5f, 0.0f}, // position
                 std::array<float, 3>{0.0f, 1.0f, 0.0f},  // color
                 std::array<float, 3>{0.0f, 0.0f, 1.0f},  // normal
                 std::array<float, 2>{1.0f, 0.0f}         // uv
             },
             {
                 std::array<float, 3>{0.0f, 0.5f, 0.0f}, // position
                 std::array<float, 3>{0.0f, 0.0f, 1.0f}, // color
                 std::array<float, 3>{0.0f, 0.0f, 1.0f}, // normal
                 std::array<float, 2>{0.5f, 1.0f}        // uv
             }}};

        di_renderer::graphics::draw_single_triangle(triangle_vertices.data(), shader_program_);
        check_gl_errors("draw_test_triangle");
    }
} // namespace di_renderer::render
