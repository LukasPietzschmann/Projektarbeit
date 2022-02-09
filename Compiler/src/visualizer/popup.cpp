#include "popup.hpp"

popup::popup(WINDOW* window) : m_panel(new_panel(window)), m_origin_win(window) {
	top_panel(m_panel);
	update_panels();
	hide();
}

popup::~popup() {
	del_panel(m_panel);
}

void popup::show() {
	show_panel(m_panel);
	update_panels();
	m_is_currently_shown = true;
}

void popup::hide() {
	hide_panel(m_panel);
	update_panels();
	m_is_currently_shown = false;
}

bool popup::toggle() {
	if(m_is_currently_shown)
		hide();
	else
		show();
	return m_is_currently_shown;
}

bool popup::is_currently_shown() const {
	return m_is_currently_shown;
}

WINDOW* popup::operator*() {
	return m_origin_win;
}