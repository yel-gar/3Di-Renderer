// NOLINTBEGIN
#pragma once
#include <epoxy/gl.h>
#include <gtkmm.h>

namespace di_renderer::render {
    class OpenGLArea final : public Gtk::GLArea {
      public:
        OpenGLArea();

      protected:
        void on_realize() override;
        void on_unrealize() override;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;

      private:
        void init_gl_resources();
        void free_gl_resources();

        GLuint vao = 0;
        GLuint vbo = 0;
    };
} // namespace di_renderer::render
// NOLINTEND
