#include "MainWindowHandler.hpp"

#include <iostream>
#include <gtkmm.h>

#include "render/OpenGLArea.hpp"

using di_renderer::ui::MainWindowHandler;
using di_renderer::render::OpenGLArea;

MainWindowHandler::MainWindowHandler() {
    builder = Gtk::Builder::create_from_file(Glib::build_filename(
        Glib::get_user_data_dir(),
        UI_LAYOUT_FILENAME)
        );
    window_ = nullptr;
    builder->get_widget("root", window_);

    load_ui();
}

void MainWindowHandler::show(Gtk::Application& app) const {
    app.add_window(*window_);
    window_->show_all();
}

void MainWindowHandler::load_ui() {
    connect_buttons();
    init_gl_area();
}

void MainWindowHandler::connect_buttons() {
    Gtk::Button* button = nullptr;

    // button_open
    builder->get_widget("button_open", button);
    button->signal_clicked().connect([this] {on_open_button_click();});

    // button_save
    builder->get_widget("button_save", button);
    button->signal_clicked().connect([this] {on_save_button_click();});
}

void MainWindowHandler::init_gl_area() const {
    const auto gl_area = Gtk::make_managed<OpenGLArea>();

    Gtk::Widget* placeholder_box = nullptr;
    builder->get_widget(Glib::ustring("gl_placeholder"), placeholder_box);

    if (const auto box = dynamic_cast<Gtk::Box*>(placeholder_box)) {
        box->pack_start(*gl_area);
    } else {
        std::cerr << "Error: Box is not a Gtk::Box" << std::endl;
    }
}

void MainWindowHandler::on_open_button_click() {
    // TODO
}

void MainWindowHandler::on_save_button_click() {
    // TODO
}
