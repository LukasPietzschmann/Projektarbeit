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
	if(m_scroll_y == 0 && delta < 0)
		return;
	if(!(m_scroll_y + m_height > m_max_y && delta < 0)) {
		if(m_max_y < m_height)
			return;
		if(m_scroll_y + delta > m_max_y - m_height)
			return;
	}
	m_scroll_y += delta;
	refresh();
}

WINDOW* scrollable::operator*() const {
	return m_pad;
}

void scrollable::add_string(const CH::str& string, int x, int y) {
	mvwaddnstr(m_pad, y, x, &string.elems[0], *string);
	m_max_y = std::max((uint32_t) y, m_max_y);
}

void scrollable::add_char(char c, int x, int y) {
	mvwaddch(m_pad, y, x, c);
	m_max_y = std::max((uint32_t) y, m_max_y);
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

	uint32_t segments_y_start;
	uint32_t number_of_segments_to_draw;

	if(uint32_t internal_max_x = std::max(m_max_y, m_scroll_y + m_height); internal_max_x > m_height) {
		number_of_segments_to_draw = std::max((uint32_t) 1, m_height * m_height / internal_max_x);
		segments_y_start = (m_height - number_of_segments_to_draw) *
				std::clamp(m_scroll_y, (uint32_t) 0, internal_max_x - m_height) / (internal_max_x - m_height);
		had_segments = true;
	}else {
		number_of_segments_to_draw = 0;
		had_segments = true;
	}

	for(int i = 0; had_segments && i < m_height * 4; ++i)
		mvwaddch(m_pad, i, m_width - 1, ' ');

	for(int i = 0; i < number_of_segments_to_draw; ++i)
		mvwaddch(m_pad, segments_y_start + i + m_scroll_y, m_width - 1, '|');

	prefresh(m_pad, m_scroll_y, 0, m_y_start, m_x_start, m_y_start + m_height, m_x_start + m_width);
}

uint32_t scrollable::get_width() const {
	return m_width;
}

uint32_t scrollable::get_height() const {
	return m_height;
}