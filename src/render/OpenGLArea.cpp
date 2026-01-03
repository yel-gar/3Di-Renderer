// NOLINTBEGIN please enable linting later
#include "OpenGLArea.hpp"

#include "core/AppData.hpp"

#include <iostream>

// this code was written by chatgpt but yaroslav should manage this idk what this does

using di_renderer::render::OpenGLArea;

OpenGLArea::OpenGLArea() {
    set_has_depth_buffer(true);
    set_required_version(3, 3);

    // mouse events
    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);

    // keyboard events
    add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
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

bool OpenGLArea::on_key_press_event(GdkEventKey* event) {
    m_pressed_keys.insert(event->keyval);
    parse_keyboard_movement();
    return true;
}

bool OpenGLArea::on_key_release_event(GdkEventKey* event) {
    m_pressed_keys.erase(event->keyval);
    parse_keyboard_movement();
    return true;
}

void OpenGLArea::parse_keyboard_movement() {
    math::Vector3 vec;

    // i don't know how which axis goes to which vector component so please figure it out
    if (key_pressed(GDK_KEY_w))
        vec.z += 1.f;
    if (key_pressed(GDK_KEY_s))
        vec.z -= 1.f;
    if (key_pressed(GDK_KEY_a))
        vec.x -= 1.f;
    if (key_pressed(GDK_KEY_d))
        vec.x += 1.f;
    if (key_pressed(GDK_KEY_q))
        vec.y -= 1.f;
    if (key_pressed(GDK_KEY_e))
        vec.y += 1.f;

    // zero
    if (vec.length() <= std::numeric_limits<float>::epsilon()) {
        return;
    }

    m_app_data.get_current_camera().move(vec.normalized());
}

bool OpenGLArea::key_pressed(unsigned int key) {
    return m_pressed_keys.find(key) != m_pressed_keys.end();
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

    set_can_focus(true);
    grab_focus();
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
