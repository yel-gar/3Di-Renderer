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

        bool on_button_press_event(GdkEventButton* event) override;
        bool on_button_release_event(GdkEventButton* event) override;
        bool on_motion_notify_event(GdkEventMotion* event) override;

      private:
        void init_gl_resources();
        void free_gl_resources();

        GLuint vao = 0;
        GLuint vbo = 0;

        core::AppData m_app_data{};

        bool m_dragging = false;
        double m_last_x;
        double m_last_y;
    };
} // namespace di_renderer::render
// NOLINTEND
