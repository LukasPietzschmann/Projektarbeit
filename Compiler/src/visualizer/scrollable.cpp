#include "scrollable.hpp"

scrollable::scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start) :
		m_width(width), m_screen_height(height), m_x_start(x_start), m_y_start(y_start) {
	m_pad = newpad(height * 4, width * 4);
	prepare_refresh();
}

scrollable::~scrollable() {
	werase(m_pad);
	delwin(m_pad);
}

void scrollable::scroll_y(int delta) {
	if(m_scroll_y == 0 && delta < 0)
		return;
	if(!(m_scroll_y + m_screen_height > m_content_height + m_content_start_y && delta < 0)) {
		if(m_content_height + m_content_start_y < m_screen_height)
			return;
		if(m_scroll_y + delta > (m_content_height + m_content_start_y) - m_screen_height)
			return;
	}
	m_scroll_y += delta;
	prepare_refresh();
}

WINDOW* scrollable::operator*() const {
	return m_pad;
}

void scrollable::add_string(const CH::str& string, int x, int y) {
	mvwaddnstr(m_pad, y, x, &string.elems[0], *string);
	if(y < m_content_start_y) {
		const int diff = m_content_start_y - y;
		m_content_start_y -= diff;
		m_content_height += diff;
	}else if(y >= m_content_height + m_content_start_y)
		m_content_height = y - m_content_start_y;
}

void scrollable::add_char(char c, int x, int y) {
	mvwaddch(m_pad, y, x, c);
	m_content_height = std::max((uint32_t) y, m_content_height) - m_content_start_y;
}

void scrollable::del_line(int x, int y) {
	wmove(m_pad, y, x);
	//wclrtobot(m_pad);
	wclrtoeol(m_pad);
	if(y == m_content_height + m_content_start_y)
		--m_content_height;
	else if(y == m_content_start_y) {
		++m_content_start_y;
		--m_content_height;
	}
}

void scrollable::clear() {
	werase(m_pad);
	m_content_height = 0;
	m_content_start_y = 0;
}

void scrollable::prepare_refresh() {
	static bool had_segments = false;

	uint32_t segments_y_start;
	uint32_t number_of_segments_to_draw;

	if(uint32_t internal_max_x = std::max(m_content_height, m_scroll_y + m_screen_height);
			internal_max_x >
					m_screen_height) {
		number_of_segments_to_draw = std::max((uint32_t) 1, m_screen_height * m_screen_height / internal_max_x);
		segments_y_start = (m_screen_height - number_of_segments_to_draw) *
				std::clamp(m_scroll_y, (uint32_t) 0, internal_max_x - m_screen_height) /
				(internal_max_x - m_screen_height);
		had_segments = true;
	}else {
		number_of_segments_to_draw = 0;
		had_segments = false;
	}

	for(int i = 0; had_segments && i < m_screen_height * 4; ++i)
		mvwaddch(m_pad, i, m_width - 1, ' ');

	for(int i = 0; i < number_of_segments_to_draw; ++i)
		mvwaddch(m_pad, segments_y_start + i + m_scroll_y, m_width - 1, '|');

	pnoutrefresh(m_pad, m_scroll_y + m_content_start_y, 0, m_y_start, m_x_start, m_y_start + m_screen_height,
			m_x_start + m_width);
}

uint32_t scrollable::get_width() const {
	return m_width;
}

uint32_t scrollable::get_height() const {
	return m_screen_height;
}