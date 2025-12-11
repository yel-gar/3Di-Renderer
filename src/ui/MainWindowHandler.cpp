#include "MainWindowHandler.hpp"

#include "core/AppData.hpp"
#include "core/Mesh.hpp"
#include "io/ObjReader.hpp"
#include "render/OpenGLArea.hpp"

#include <cassert>
#include <gtkmm.h>
#include <iostream>

using di_renderer::render::OpenGLArea;
using di_renderer::ui::MainWindowHandler;

MainWindowHandler::MainWindowHandler() {
    m_builder = Gtk::Builder::create_from_resource("/ru/vsu/cs/direnderer/ui/di_renderer.ui");
    m_builder->get_widget("root", m_window);

    load_ui();
}

void MainWindowHandler::show(Gtk::Application& app) const {
    app.add_window(*m_window);
    m_window->show_all();
}

void MainWindowHandler::load_ui() {
    connect_buttons();
    init_gl_area();
}

void MainWindowHandler::connect_buttons() {
    Gtk::Button* button = nullptr;

    // button_open
    m_builder->get_widget("button_open", button);
    button->signal_clicked().connect([this] { on_open_button_click(); });

    // button_save
    m_builder->get_widget("button_save", button);
    button->signal_clicked().connect([this] { on_save_button_click(); });

    // render mode toggle buttons
    Gtk::ToggleButton* toggle_button = nullptr;
    for (const auto& [button_id, render_mode] : ID_TO_MODE_MAP) {
        m_builder->get_widget(button_id, toggle_button);
        assert(toggle_button != nullptr);
        toggle_button->signal_toggled().connect(
            [toggle_button, mode = render_mode] { on_render_toggle_button_click(*toggle_button, mode); });
    }

    // texture selector
    Gtk::FileChooserButton* texture_selector = nullptr;
    m_builder->get_widget("texture_file_selector", texture_selector);
    texture_selector->signal_file_set().connect(
        [texture_selector] { on_texture_selection(texture_selector->get_filename()); });
}

void MainWindowHandler::init_gl_area() const {
    auto* const gl_area = Gtk::make_managed<OpenGLArea>();

    Gtk::Widget* placeholder_box = nullptr;
    m_builder->get_widget("gl_placeholder", placeholder_box);

    if (auto* const box = dynamic_cast<Gtk::Box*>(placeholder_box)) {
        box->pack_start(*gl_area);
    } else {
        std::cerr << "Error: Box is not a Gtk::Box" << '\n';
    }
}

void MainWindowHandler::on_open_button_click() const {
    auto dialog =
        Gtk::FileChooserNative::create("Select model", *m_window, Gtk::FILE_CHOOSER_ACTION_OPEN, "_Open", "_Cancel");

    const auto filter = Gtk::FileFilter::create();
    filter->set_name("OBJ files");
    filter->add_pattern("*.obj");
    dialog->set_filter(filter);

    dialog->signal_response().connect([dialog](const int response_id) {
        if (response_id == Gtk::RESPONSE_ACCEPT) {
            const auto filename = dialog->get_filename();
            const auto [vertices, texture_vertices, normals, faces] = io::ObjReader::read_file(filename);
            core::Mesh mesh{vertices, texture_vertices, normals, faces};
            core::AppData::instance().add_mesh(std::move(mesh));
        }
    });

    dialog->show();
}

void MainWindowHandler::on_save_button_click() const {
    // TODO
}

void MainWindowHandler::on_texture_selection(const std::string& filename) {
    auto& mesh = core::AppData::instance().get_current_mesh();
    mesh.load_texture(filename);
}

void MainWindowHandler::on_render_toggle_button_click(const Gtk::ToggleButton& btn, const core::RenderMode mode) {
    core::AppData::instance().set_render_mode(mode, btn.get_active());
}
