// NOLINTBEGIN please enable linting later
#include "OpenGLArea.hpp"

#include "core/AppData.hpp"

#include <iostream>

// this code was written by chatgpt but yaroslav should manage this idk what this does

using di_renderer::render::OpenGLArea;

OpenGLArea::OpenGLArea() {
    set_has_depth_buffer(true);
    set_required_version(3, 3);

    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

bool OpenGLArea::on_button_press_event(GdkEventButton* event) {
    if (event->button == 1) { // Left mouse button
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

    m_app_data.get_current_camera().parse_mouse_movement(dx, dy);

    return true;
}

void OpenGLArea::on_realize() {
    GLArea::on_realize();

    make_current();

    if (auto err = glGetError(); err != GL_NO_ERROR) {
        std::cerr << err << std::endl;
    }

    init_gl_resources();
}

void OpenGLArea::on_unrealize() {
    make_current();
    free_gl_resources();

    GLArea::on_unrealize();
}

di_renderer::core::AppData& OpenGLArea::get_app_data() noexcept {
    return m_app_data;
}

bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext>& context) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
}

void OpenGLArea::init_gl_resources() {
    const float vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) 0);
}

void OpenGLArea::free_gl_resources() {
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
    vbo = vao = 0;
}
// NOLINTEND
