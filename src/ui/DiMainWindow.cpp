#include "DiMainWindow.hpp"
#include "OpenGLArea.hpp"

using di_renderer::ui::DiMainWindow;
using di_renderer::ui::OpenGLArea;

DiMainWindow::DiMainWindow() {
    set_default_size(800, 600);
    set_title("Di Renderer");

    init_ui();
}

void DiMainWindow::init_ui() {
    const auto gl_area = Gtk::make_managed<OpenGLArea>();
    add(*gl_area);
    gl_area->show();
}

void DiMainWindow::replace_gl_area() {
}
