// NOLINTBEGIN
#pragma once
#include "core/AppData.hpp"

#include <epoxy/gl.h>
#include <gtkmm.h>
#include <unordered_set>

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

        bool on_key_press_event(GdkEventKey* event) override;
        bool on_key_release_event(GdkEventKey* event) override;

      private:
        void init_gl_resources();
        void free_gl_resources();
        void parse_keyboard_movement();
        bool key_pressed(unsigned int key);

        GLuint vao = 0;
        GLuint vbo = 0;

        core::AppData m_app_data{};

        bool m_dragging = false;
        double m_last_x;
        double m_last_y;

        std::unordered_set<unsigned int> m_pressed_keys;
    };
} // namespace di_renderer::render
// NOLINTEND
