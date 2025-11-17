#include "DiRendererApp.hpp"
#include "DiMainWindow.hpp"

using di_renderer::ui::DiRendererApp;
using di_renderer::ui::DiMainWindow;

Glib::RefPtr<DiRendererApp> DiRendererApp::create() {
    return Glib::RefPtr(new DiRendererApp());
}

DiRendererApp::DiRendererApp() : Application("ru.vsu.cs.direnderer") {
}

void DiRendererApp::on_startup() {
    Application::on_startup();
}

void DiRendererApp::on_activate() {
    auto* win = new DiMainWindow();
    add_window(*win);
    win->show();
}
