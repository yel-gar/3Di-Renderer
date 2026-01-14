#pragma once

#include "render/Camera.hpp"

#include <atomic>
#include <epoxy/gl.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/glarea.h>
#include <sigc++/connection.h>
#include <unordered_set>

namespace di_renderer::render {

    class OpenGLArea : public Gtk::GLArea {
      public:
        OpenGLArea();
        OpenGLArea(const OpenGLArea&) = delete;
        OpenGLArea(OpenGLArea&&) = delete;
        OpenGLArea& operator=(const OpenGLArea&) = delete;
        OpenGLArea& operator=(OpenGLArea&&) = delete;
        ~OpenGLArea() override;

        void ensure_keyboard_focus();

      protected:
        // Lifecycle events
        void on_show() override;
        void on_hide() override;
        void on_realize() final;
        void on_unrealize() final;
        void on_resize(int width, int height) final;
        void on_map() final;
        void on_unmap() final;

        // Input events
        bool on_key_press_event(GdkEventKey* key_event) override;
        bool on_key_release_event(GdkEventKey* key_event) override;
        bool on_button_press_event(GdkEventButton* button_event) override;
        bool on_button_release_event(GdkEventButton* button_event) override;
        bool on_motion_notify_event(GdkEventMotion* motion_event) override;
        bool on_focus_in_event(GdkEventFocus* focus_event) override;
        bool on_focus_out_event(GdkEventFocus* focus_event) override;

        // Rendering
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) final;

      private:
        // Camera management
        void update_camera_for_mesh();
        void reset_camera_for_new_model();

        // Input handling
        void parse_keyboard_movement();
        bool key_pressed(unsigned int key);

        // Rendering helpers
        void set_default_uniforms();
        void draw_current_mesh();
        void update_dynamic_projection();
        // Animation management
        void on_dispatch_render();
        void start_animation();
        bool on_animation_timeout();
        void stop_animation();
        void cleanup_resources();

        // GL state
        Glib::Dispatcher m_render_dispatcher;
        sigc::connection m_render_connection;
        std::atomic<bool> m_gl_initialized{false};
        std::atomic<bool> m_should_render{false};
        GLuint m_shader_program = 0;
        float m_last_frame_time = 0.0f;

        // Camera
        Camera m_camera;
        float m_movement_speed = 2.5f;

        // Input state
        bool m_has_focus = false;
        std::unordered_set<guint> m_pressed_keys;

        // Mouse drag state
        bool m_dragging = false;
        double m_last_x = 0.0;
        double m_last_y = 0.0;
    };

} // namespace di_renderer::render
