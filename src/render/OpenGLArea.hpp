// NOLINTBEGIN
#pragma once
#include "core/AppData.hpp"

#include <epoxy/gl.h>
#include <gtkmm.h>

namespace di_renderer::render {
    class OpenGLArea final : public Gtk::GLArea {
      public:
        OpenGLArea();

        core::AppData& get_app_data() noexcept;

      protected:
        void on_realize() override;
        void on_unrealize() override;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;

      private:
        void init_gl_resources();
        void free_gl_resources();

        GLuint vao = 0;
        GLuint vbo = 0;

        core::AppData m_app_data{};
    };
} // namespace di_renderer::render
// NOLINTEND
