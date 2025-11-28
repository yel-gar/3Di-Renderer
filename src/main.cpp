#include "ui/DiRendererApp.hpp"

int main(int argc, char** argv) {
    const auto res = Gio::Resource::create_from_file(
        Glib::build_filename(Glib::get_user_data_dir(), "direnderer/gresources.gresource"));
    res->register_global();

    const auto app = di_renderer::ui::DiRendererApp::create();
    return app->run(argc, argv);
}
