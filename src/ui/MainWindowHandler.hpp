#pragma once
#include "core/RenderMode.hpp"

#include <gtkmm.h>

namespace di_renderer::ui {
    class MainWindowHandler final {
      public:
        MainWindowHandler();

        void show(Gtk::Application& app) const;

      private:
        inline static const std::string UI_LAYOUT_FILENAME{"direnderer/ui/di_renderer.ui"};
        inline static const std::unordered_map<std::string, core::RenderMode> ID_TO_MODE_MAP{
            {"button_toggle_polygons", core::RenderMode::POLYGON},
            {"button_toggle_texture", core::RenderMode::TEXTURE},
            {"button_toggle_lighting", core::RenderMode::LIGHTING}};

        Glib::RefPtr<Gtk::Builder> m_builder;
        Gtk::Window* m_window{nullptr};
        Gtk::FileChooserButton* m_texture_selector{nullptr};

        void load_ui();
        void connect_buttons();
        void init_error_handling() const;
        void init_gl_area() const;
        void on_open_button_click() const;
        void on_save_button_click() const;
        void on_texture_selection() const;
        static void on_render_toggle_button_click(const Gtk::ToggleButton& btn, core::RenderMode mode);
    };
} // namespace di_renderer::ui
