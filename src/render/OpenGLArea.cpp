#include "OpenGLArea.hpp"
#include <iostream>

// this code was written by chatgpt but yaroslav should manage this idk what this does

using di_renderer::render::OpenGLArea;


OpenGLArea::OpenGLArea() {
    set_has_depth_buffer(true);
    set_required_version(3, 3);
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

bool OpenGLArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
}

void OpenGLArea::init_gl_resources() {
    const float vertices[] = {
        0.0f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}

void OpenGLArea::free_gl_resources() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    vbo = vao = 0;
}
