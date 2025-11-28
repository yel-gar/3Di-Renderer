#pragma once
#include <gtkmm.h>

namespace di_renderer::ui {
    class MainWindowHandler final {
      public:
        MainWindowHandler();

        void show(Gtk::Application& app) const;

      private:
        inline static const std::string UI_LAYOUT_FILENAME{"direnderer/ui/di_renderer.ui"};

        Glib::RefPtr<Gtk::Builder> m_builder;
        Gtk::Window* m_window{nullptr};

        void load_ui();
        void connect_buttons();
        void init_gl_area() const;
        void on_open_button_click() const;
        void on_save_button_click();
    };
} // namespace di_renderer::ui
