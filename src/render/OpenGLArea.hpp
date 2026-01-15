#pragma once

#include "TextureLoader.hpp"
#include "core/AppData.hpp"
#include "glibmm/dispatcher.h"
#include "glibmm/main.h"
#include "math/Camera.hpp"
#include "math/Transform.hpp"
#include "render/TextureLoader.hpp"

#include <atomic>
#include <epoxy/gl_generated.h>
#include <gtkmm/glarea.h>
#include <gtkmm/main.h>
#include <sigc++/sigc++.h>
#include <unordered_set>

namespace di_renderer::render {

    class OpenGLArea : public Gtk::GLArea {
      public:
        OpenGLArea();
        ~OpenGLArea() override;
        OpenGLArea(const OpenGLArea&) = delete;
        OpenGLArea& operator=(const OpenGLArea&) = delete;
        OpenGLArea(OpenGLArea&&) = delete;
        OpenGLArea& operator=(OpenGLArea&&) = delete;

        void reset_camera_for_new_model();
        di_renderer::core::AppData& get_app_data() noexcept;
        void set_current_mesh_path(const std::string& path);

      protected:
        // Widget overrides
        void on_realize() override;
        void on_unrealize() override;
        void on_resize(int width, int height) override;
        bool on_render(const Glib::RefPtr<Gdk::GLContext>& context) override;
        bool on_button_press_event(GdkEventButton* event) override;
        bool on_button_release_event(GdkEventButton* event) override;
        bool on_motion_notify_event(GdkEventMotion* event) override;
        bool on_key_press_event(GdkEventKey* event) override;
        bool on_key_release_event(GdkEventKey* event) override;
        void on_map() override;
        void on_unmap() override;

      private:
        void start_animation();
        void stop_animation();
        bool on_animation_timeout();
        void on_dispatch_render();
        void cleanup_resources();
        void parse_keyboard_movement();
        bool key_pressed(unsigned int key);
        void update_camera_for_mesh();
        void update_dynamic_projection();
        void set_default_uniforms();
        void draw_current_mesh();
        void draw_wireframe_overlay();

        void calculate_camera_planes(const di_renderer::math::Vector3& min_pos,
                                     const di_renderer::math::Vector3& max_pos, float distance,
                                     di_renderer::math::Camera& camera);
        di_renderer::math::Vector3 transform_vertex(const di_renderer::math::Vector3& vertex,
                                                    const di_renderer::math::Transform& transform);

        di_renderer::core::AppData m_app_data;
        di_renderer::graphics::TextureLoader m_texture_loader;
        GLuint m_shader_program = 0;
        double m_last_x, m_last_y;
        bool m_dragging = false;
        std::unordered_set<unsigned int> m_pressed_keys;
        std::atomic<bool> m_gl_initialized{false};
        std::atomic<bool> m_should_render{false};
        Glib::RefPtr<Glib::MainContext> m_main_context;
        sigc::connection m_render_connection;
        Glib::Dispatcher m_render_dispatcher;
        bool m_flip_uv_y = true;
        std::string m_current_mesh_path;
    };

} // namespace di_renderer::render
