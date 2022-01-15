#include "scrollable.hpp"

scrollable::scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start) :
		m_width(width), m_height(height), m_x_start(x_start), m_y_start(y_start) {
	m_pad = newpad(height * 4, width * 4);
	refresh();
}

scrollable::~scrollable() {
	werase(m_pad);
	delwin(m_pad);
}

void scrollable::scroll_y(int delta) {
	if(m_max_y < m_height)
		return;
	if(m_scroll_y + delta > m_max_y - m_height)
		return;
	m_scroll_y += delta;
	refresh();
}

WINDOW* scrollable::operator*() const {
	return m_pad;
}

void scrollable::project_window(WINDOW* win) {
	assert(win != nullptr);
	overwrite(win, m_pad);
	int height = getmaxy(win);
	int y_start = getbegy(win);
	m_max_y = std::max(y_start + height, m_max_y);
}

void scrollable::add_string(const CH::str& string, int x, int y) {
	mvwaddnstr(m_pad, y, x, &string.elems[0], *string);
	m_max_y = std::max(y, m_max_y);
}

void scrollable::del_line(int x, int y) {
	wmove(m_pad, y, x);
	wclrtobot(m_pad);
	if(y == m_max_y)
		--m_max_y;
}

void scrollable::clear() {
	werase(m_pad);
	m_max_y = 0;
}

void scrollable::refresh() {
	static bool had_segments = false;

	int segments_to_draw = m_height - (m_max_y - m_height);
	if(segments_to_draw > m_height) {
		segments_to_draw = 0;
		had_segments = false;
	}else
		had_segments = true;

	for(int i = 0; had_segments && i < m_height * 4; ++i)
		mvwaddch(m_pad, i, m_width - 1, ' ');

	for(int i = 0; i < segments_to_draw; ++i)
		mvwaddch(m_pad, m_scroll_y * 2 + i, m_width - 1, '|');

	prefresh(m_pad, m_scroll_y, 0, m_y_start, m_x_start, m_y_start + m_height, m_x_start + m_width);
}

uint32_t scrollable::get_width() const {
	return m_width;
}

uint32_t scrollable::get_height() const {
	return m_height;
}