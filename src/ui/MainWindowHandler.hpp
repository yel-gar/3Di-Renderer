#pragma once
#include "CameraSelectionHandler.hpp"
#include "core/RenderMode.hpp"
#include "render/OpenGLArea.hpp"

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
        inline static const std::array<std::string, 9> TRANSFORM_IDS = {
            "translation_x", "translation_y", "translation_z", "rotation_x", "rotation_y",
            "rotation_z",    "scale_x",       "scale_y",       "scale_z"};

        CameraSelectionHandler m_camera_selection_handler;
        Glib::RefPtr<Gtk::Builder> m_builder;
        Gtk::Window* m_window{nullptr};
        Gtk::FileChooserButton* m_texture_selector{nullptr};
        render::OpenGLArea* m_gl_area{nullptr};

        void load_ui();
        void connect_buttons();
        void connect_entries();
        void init_error_handling() const;
        void init_gl_area();
        void on_open_button_click() const;
        void on_save_button_click() const;
        void on_texture_selection() const;
        void on_render_toggle_button_click(const Gtk::ToggleButton& btn, core::RenderMode mode);
        void on_transform_entry_activate(Gtk::Entry& entry, const std::string& entry_id);
    };
} // namespace di_renderer::ui
