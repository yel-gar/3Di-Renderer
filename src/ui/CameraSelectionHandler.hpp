#pragma once
#include "core/AppData.hpp"

#include <gtkmm.h>

namespace di_renderer::ui {
    class CameraSelectionHandler final {
      public:
        void init(const Glib::RefPtr<Gtk::Builder>& builder, core::AppData& app_data);

      private:
        Glib::RefPtr<Gtk::ListStore> m_store{nullptr};
        Gtk::TreeView* m_view{nullptr};
        Gtk::Button* m_delete_button{nullptr};
        core::AppData* m_app_data{nullptr};
        unsigned int m_max_index = 0;

        void on_add_button_click();
        void on_delete_button_click();
        void on_selection();
    };
} // namespace di_renderer::ui
