#pragma once

#include "Triangle.hpp"

#include <atomic>
#include <glibmm/dispatcher.h>
#include <gtkmm/glarea.h>
#include <sigc++/sigc++.h>

namespace di_renderer {
    namespace render {

        class OpenGLArea : public Gtk::GLArea {
          public:
            OpenGLArea();
            ~OpenGLArea() override;

          protected:
            bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;
            void on_realize() override;
            void on_unrealize() override;
            void on_resize(int width, int height) override;
            void on_map() override;
            void on_unmap() override;

          private:
            sigc::connection render_connection_;
            Glib::Dispatcher render_dispatcher_;
            std::atomic<bool> gl_initialized_{false};
            std::atomic<bool> should_render_{false};
            GLuint shader_program_ = 0;
            float rotation_angle_ = 0.0f;
            float last_frame_time_ = 0.0f;

            void init_resources();
            void cleanup_resources();
            void set_default_uniforms();
            void draw_test_cube();
            bool on_animation_timeout();
            void start_animation();
            void stop_animation();
            void on_dispatch_render();
        };

    } // namespace render
} // namespace di_renderer
