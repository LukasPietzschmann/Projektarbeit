#include "popup.hpp"

popup::popup(scrollable* window) : window_like<scrollable>(window), m_panel(new_panel(**window)) {
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

void popup::add_n_str(const CH::str& str, int x, int y) {
	mvsaddstr(m_underlying_window, y, x, str);
}

void popup::del_line(int x, int y) {
	m_underlying_window->del_line(x, y);
}

void popup::clear() {
	m_underlying_window->clear();
}

void popup::prepare_refresh() const {
	m_underlying_window->prepare_refresh();
	update_panels();
}