#pragma once

#include "Triangle.hpp"

#include <gtkmm/glarea.h>

namespace di_renderer::render {

    class OpenGLArea : public Gtk::GLArea {
      public:
        OpenGLArea();
        ~OpenGLArea() override;

      protected:
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;
        void on_realize() override;
        void on_unrealize() override;
        void on_resize(int width, int height) override;

      private:
        GLuint shader_program_ = 0;
        void init_resources();
        void cleanup_resources();
        void set_default_uniforms();
        void draw_test_triangle();
        void check_gl_errors(const char* operation) const;
    };

} // namespace di_renderer::render
