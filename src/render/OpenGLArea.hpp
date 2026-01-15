#pragma once

#include "core/AppData.hpp"

#include <atomic>
#include <epoxy/gl.h>
#include <glibmm/dispatcher.h>
#include <gtkmm.h>
#include <gtkmm/glarea.h>
#include <sigc++/connection.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace di_renderer::render {
    class OpenGLArea final : public Gtk::GLArea {
      public:
        OpenGLArea();
        ~OpenGLArea() override;

        core::AppData& get_app_data() noexcept;

      protected:
        void on_realize() override;
        void on_unrealize() override;
        void on_resize(int width, int height) override;
        void on_map() override;
        void on_unmap() override;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;
        bool on_button_press_event(GdkEventButton* event) override;
        bool on_button_release_event(GdkEventButton* event) override;
        bool on_motion_notify_event(GdkEventMotion* event) override;
        bool on_key_press_event(GdkEventKey* event) override;
        bool on_key_release_event(GdkEventKey* event) override;

      private:
        void update_camera_for_mesh();
        void reset_camera_for_new_model();
        void parse_keyboard_movement();
        bool key_pressed(unsigned int key);
        void set_default_uniforms();
        void draw_current_mesh();
        void update_dynamic_projection();
        void on_dispatch_render();
        void start_animation();
        bool on_animation_timeout();
        void stop_animation();
        void cleanup_resources();
        void resolve_texture_path(std::string& texture_path, const std::string& base_path);
        GLuint load_texture_from_file(const std::string& filename, const std::string& base_path);

        Glib::Dispatcher m_render_dispatcher;
        sigc::connection m_render_connection;
        std::atomic<bool> m_gl_initialized{false};
        std::atomic<bool> m_should_render{false};
        GLuint m_shader_program = 0;
        core::AppData m_app_data;
        bool m_dragging = false;
        double m_last_x;
        double m_last_y;
        std::unordered_set<unsigned int> m_pressed_keys;
        std::unordered_map<std::string, GLuint> m_loaded_textures;
        std::string m_current_mesh_path;
        bool m_flip_uv_y = true;
    };
} // namespace di_renderer::render
