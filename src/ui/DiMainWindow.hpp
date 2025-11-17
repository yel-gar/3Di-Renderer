#pragma once
#include <gtkmm.h>

namespace di_renderer::ui {
    class DiMainWindow : public Gtk::Window {
    public:
        DiMainWindow();

    private:
        Glib::RefPtr<Gtk::Builder> builder;

        void init_ui();
        void replace_gl_area();
    };
}
