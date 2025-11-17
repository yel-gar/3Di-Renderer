#pragma once
#include <gtkmm.h>
#include <epoxy/gl.h>

namespace di_renderer::ui {
class OpenGLArea final : public Gtk::GLArea {
public:
    OpenGLArea();

protected:
    void on_realize() override;
    void on_unrealize() override;
    bool on_render(const Glib::RefPtr<Gdk::GLContext> &context) override;

private:
    void init_gl_resources();
    void free_gl_resources();

    GLuint vao = 0;
    GLuint vbo = 0;
};
} // di_renderer
