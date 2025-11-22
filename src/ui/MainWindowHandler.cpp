#include "MainWindowHandler.hpp"

#include "render/OpenGLArea.hpp"

#include <gtkmm.h>
#include <iostream>

using di_renderer::render::OpenGLArea;
using di_renderer::ui::MainWindowHandler;

MainWindowHandler::MainWindowHandler()
{
    m_builder = Gtk::Builder::create_from_file(Glib::build_filename(Glib::get_user_data_dir(), UI_LAYOUT_FILENAME));
    m_builder->get_widget("root", m_window);

    load_ui();
}

void MainWindowHandler::show(Gtk::Application& app) const
{
    app.add_window(*m_window);
    m_window->show_all();
}

void MainWindowHandler::load_ui()
{
    connect_buttons();
    init_gl_area();
}

void MainWindowHandler::connect_buttons()
{
    Gtk::Button* button = nullptr;

    // button_open
    m_builder->get_widget("button_open", button);
    button->signal_clicked().connect([this] { on_open_button_click(); });

    // button_save
    m_builder->get_widget("button_save", button);
    button->signal_clicked().connect([this] { on_save_button_click(); });
}

void MainWindowHandler::init_gl_area() const
{
    auto* const gl_area = Gtk::make_managed<OpenGLArea>();

    Gtk::Widget* placeholder_box = nullptr;
    m_builder->get_widget("gl_placeholder", placeholder_box);

    if (auto* const box = dynamic_cast<Gtk::Box*>(placeholder_box))
    {
        box->pack_start(*gl_area);
    }
    else
    {
        std::cerr << "Error: Box is not a Gtk::Box" << '\n';
    }
}

void MainWindowHandler::on_open_button_click()
{
    // TODO
}

void MainWindowHandler::on_save_button_click()
{
    // TODO
}
