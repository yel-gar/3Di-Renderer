#include "MainWindowHandler.hpp"

#include "TransformTypeHelper.hpp"
#include "core/AppData.hpp"
#include "core/Mesh.hpp"
#include "io/ObjReader.hpp"
#include "io/ObjWriter.hpp"
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
    init_error_handling();
}

void MainWindowHandler::show(Gtk::Application& app) const {
    app.add_window(*m_window);
    m_window->show_all();
}

void MainWindowHandler::load_ui() {
    connect_buttons();
    connect_entries();
    init_gl_area();
    m_camera_selection_handler.init(m_builder, m_gl_area->get_app_data());
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
            [this, toggle_button, mode = render_mode] { on_render_toggle_button_click(*toggle_button, mode); });
    }

    // texture selector
    m_builder->get_widget("texture_file_selector", m_texture_selector);
    m_texture_selector->signal_file_set().connect([this] { on_texture_selection(); });
}

void MainWindowHandler::connect_entries() {
    Gtk::Entry* entry = nullptr;

    for (const auto& entry_id : TRANSFORM_IDS) {
        m_builder->get_widget(entry_id, entry);
        assert(entry != nullptr);
        entry->signal_activate().connect([this, entry, id = entry_id] { on_transform_entry_activate(*entry, id); });
    }
}

void MainWindowHandler::init_error_handling() const {
    auto handler = [this] {
        try {
            throw; // Re-throw the current exception
        } catch (const std::exception& ex) {
            const std::string error_msg = ex.what();
            std::cerr << "[ERROR] " << error_msg << '\n';

            Glib::signal_idle().connect_once([this, error_msg] {
                Gtk::MessageDialog dialog(*m_window, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                dialog.set_secondary_text(error_msg);
                dialog.run();
            });
        } catch (...) {
            std::cerr << "[ERROR] Unknown exception" << '\n';

            Glib::signal_idle().connect_once([this] {
                Gtk::MessageDialog dialog(*m_window, "Error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                dialog.set_secondary_text("An unknown error occurred");
                dialog.run();
            });
        }
    };

    Glib::add_exception_handler(handler);
}

void MainWindowHandler::init_gl_area() {
    m_gl_area = Gtk::make_managed<OpenGLArea>();

    Gtk::Widget* placeholder_box = nullptr;
    m_builder->get_widget("gl_placeholder", placeholder_box);

    if (auto* const box = dynamic_cast<Gtk::Box*>(placeholder_box)) {
        box->pack_start(*m_gl_area);
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

    dialog->signal_response().connect([this, dialog](const int response_id) {
        if (response_id == Gtk::RESPONSE_ACCEPT) {
            const auto filename = dialog->get_filename();
            const auto [vertices, texture_vertices, normals, faces] = io::ObjReader::read_file(filename);
            core::Mesh mesh{vertices, texture_vertices, normals, faces};
            m_gl_area->get_app_data().add_mesh(std::move(mesh));

            Gtk::Entry* entry = nullptr;
            for (const auto& entry_id : TRANSFORM_IDS) {
                m_builder->get_widget(entry_id, entry);
                entry->set_text(entry_id.substr(0, entry_id.find('_')) == "scale" ? "1" : "0");
            }
        }
    });

    dialog->show();
}

void MainWindowHandler::on_save_button_click() const {
    auto dialog =
        Gtk::FileChooserNative::create("Select model", *m_window, Gtk::FILE_CHOOSER_ACTION_SAVE, "_Save", "_Cancel");

    const auto filter = Gtk::FileFilter::create();
    filter->set_name("OBJ files");
    filter->add_pattern("*.obj");
    dialog->set_filter(filter);

    dialog->signal_response().connect([this, dialog](const int response_id) {
        if (response_id == Gtk::RESPONSE_ACCEPT) {
            const auto& filename = dialog->get_filename();
            const auto& mesh = m_gl_area->get_app_data().get_current_mesh();
            io::ObjWriter::write_file(filename, mesh);
        }
    });

    dialog->show();
}

void MainWindowHandler::on_texture_selection() const {
    auto& mesh = m_gl_area->get_app_data().get_current_mesh();
    mesh.load_texture(m_texture_selector->get_filename());
}

void MainWindowHandler::on_render_toggle_button_click(const Gtk::ToggleButton& btn, const core::RenderMode mode) {
    m_gl_area->get_app_data().set_render_mode(mode, btn.get_active());
}

void MainWindowHandler::on_transform_entry_activate(Gtk::Entry& entry, const std::string& entry_id) {
    core::Mesh* mesh_ptr = nullptr;
    try {
        mesh_ptr = &m_gl_area->get_app_data().get_current_mesh();
    } catch (const std::exception&) {
        std::cout << "No active mesh, ignoring entry value set";
        return;
    }

    auto& transform = mesh_ptr->get_transform(); // safe dereference
    const std::string text = entry.get_text();
    const auto transform_type = get_transform_type(entry_id);

    float value;
    try {
        value = std::stof(text);
    } catch (const std::exception& e) {
        std::cout << "Bad numeric value: " << e.what() << '\n';
        return;
    }

    // special case for scale invalid input
    if (transform_type.type == TransformType::SCALE && value <= std::numeric_limits<float>::epsilon()) {
        entry.set_text(std::to_string(get_vector_component(transform.get_scale(), transform_type.component)));
        return;
    }

    // ugly code incoming
    switch (transform_type.type) {
    case TransformType::TRANSLATE:
        transform.set_position(get_new_vector(transform.get_position(), transform_type.component, value));
        break;
    case TransformType::ROTATE:
        transform.set_rotation(get_new_vector(transform.get_rotation(), transform_type.component, value));
        break;
    case TransformType::SCALE:
        transform.set_scale(get_new_vector(transform.get_scale(), transform_type.component, value));
        break;
    }
}
