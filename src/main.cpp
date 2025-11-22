#include "ui/DiRendererApp.hpp"

int main(int argc, char** argv)
{
    const auto app = di_renderer::ui::DiRendererApp::create();
    return app->run(argc, argv);
}
