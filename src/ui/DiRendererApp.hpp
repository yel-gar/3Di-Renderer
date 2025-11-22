#pragma once
#include <gtkmm.h>

namespace di_renderer::ui
{
    class DiRendererApp final : public Gtk::Application
    {
      public:
        static Glib::RefPtr<DiRendererApp> create();

      protected:
        DiRendererApp();

        void on_startup() override;
        void on_activate() override;
    };
} // namespace di_renderer::ui
