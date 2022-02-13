#include "popup.hpp"

popup::popup(scrollable* window) : m_panel(new_panel(**window)), m_origin_win(window) {
	top_panel(m_panel);
	update_panels();
	hide();
}

popup::~popup() {
	del_panel(m_panel);
}

void popup::show() {
	show_panel(m_panel);
	prepare_refresh();
	doupdate();
	m_is_currently_shown = true;
}

void popup::hide() {
	hide_panel(m_panel);
	update_panels();
	m_is_currently_shown = false;
}

bool popup::is_currently_shown() const {
	return m_is_currently_shown;
}

void popup::prepare_refresh() const {
	m_origin_win->prepare_refresh();
	update_panels();
}

scrollable* popup::operator*() {
	return m_origin_win;
}