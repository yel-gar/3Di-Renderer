#pragma once

#include <atomic>
#include <epoxy/gl.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/glarea.h>
#include <sigc++/connection.h>

namespace di_renderer {
    namespace render {

        class OpenGLArea : public Gtk::GLArea {
          public:
            OpenGLArea();
            ~OpenGLArea() override;

          protected:
            void on_realize() override final;
            void on_unrealize() override final;
            void on_resize(int width, int height) override final;
            void on_map() override final;
            void on_unmap() override final;
            bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override final;

          private:
            void on_dispatch_render();
            void start_animation();
            bool on_animation_timeout();
            void stop_animation();
            void cleanup_resources();
            void set_default_uniforms();
            void draw_current_mesh();

            Glib::Dispatcher render_dispatcher_;
            sigc::connection render_connection_;

            std::atomic<bool> gl_initialized_{false};
            std::atomic<bool> should_render_{false};

            GLuint shader_program_ = 0;
            float rotation_angle_ = 0.0f;
            float last_frame_time_ = 0.0f;
        };

    } // namespace render
} // namespace di_renderer
