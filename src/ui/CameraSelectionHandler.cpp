#include "CameraSelectionHandler.hpp"

#include <iostream>
void di_renderer::ui::CameraSelectionHandler::init(const Glib::RefPtr<Gtk::Builder>& builder, core::AppData& app_data) {
    m_app_data = &app_data;

    auto store_obj = builder->get_object("camera_list_store");
    m_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(store_obj);

    builder->get_widget("camera_tree_view", m_view);

    // add default camera
    auto row = m_store->append();
    row->set_value(0, 0);

    // select camera 0
    const auto path = m_store->get_path(row);
    m_view->get_selection()->select(row);
    m_view->scroll_to_row(path);

    // connect signal for selection
    m_view->get_selection()->signal_changed().connect([this] { on_selection(); });

    Gtk::Button* btn = nullptr;
    builder->get_widget("camera_add", btn);
    btn->signal_clicked().connect([this] { on_add_button_click(); });
    builder->get_widget("camera_delete", btn);
    btn->signal_clicked().connect([this] { on_delete_button_click(); });

    m_delete_button = btn;
}

void di_renderer::ui::CameraSelectionHandler::on_add_button_click() {
    m_delete_button->set_sensitive(true);

    auto row = *m_store->append();
    row.set_value(0, ++m_max_index);

    const auto path = m_store->get_path(row);
    m_view->get_selection()->select(row);
    m_view->scroll_to_row(path);
}

void di_renderer::ui::CameraSelectionHandler::on_delete_button_click() {
    m_app_data->delete_current_camera();

    // ensure can't delete last camera
    if (m_store->children().size() == 2) {
        m_delete_button->set_sensitive(false);
    }

    // chatgpt code im lazy
    const auto sel = m_view->get_selection()->get_selected();
    if (!sel) {
        return;
    }

    // Prefer next row
    auto next = sel;
    ++next;

    // If no next row, select previous instead
    if (!next) {
        if (sel != m_store->children().begin()) {
            next = sel;
            --next;
        }
    }

    if (next) {
        m_view->get_selection()->select(next);
    }

    m_store->erase(sel);
}

void di_renderer::ui::CameraSelectionHandler::on_selection() {
    unsigned int selected_id = 0;
    const auto sel = m_view->get_selection()->get_selected();
    if (!sel) {
        std::cerr << "Warning: there's no selection but on_selection was called in camera tree selector\n";
    }
    sel->get_value(0, selected_id);
    m_app_data->set_current_camera(selected_id);
}
