#include "DiRendererApp.hpp"

#include "MainWindowHandler.hpp"

using di_renderer::ui::DiRendererApp;
using di_renderer::ui::MainWindowHandler;

Glib::RefPtr<DiRendererApp> DiRendererApp::create()
{
    return Glib::RefPtr(new DiRendererApp());
}

DiRendererApp::DiRendererApp() : Application("ru.vsu.cs.direnderer") {}

void DiRendererApp::on_startup()
{
    Application::on_startup();
}

void DiRendererApp::on_activate()
{
    const MainWindowHandler handler{};
    handler.show(*this);
}
