#pragma once
#include "MainWindowHandler.hpp"

#include <gtkmm.h>

namespace di_renderer::ui {
    class DiRendererApp final : public Gtk::Application {
      public:
        static Glib::RefPtr<DiRendererApp> create();

      protected:
        DiRendererApp();

        void on_startup() override;
        void on_activate() override;

      private:
        std::unique_ptr<MainWindowHandler> m_handler;
    };
} // namespace di_renderer::ui
