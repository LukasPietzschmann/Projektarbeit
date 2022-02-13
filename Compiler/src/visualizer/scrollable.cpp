#include "scrollable.hpp"

scrollable::scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start) :
		m_width(width), m_screen_height(height), m_x_start(x_start), m_y_start(y_start) {
	m_pad = newpad(height * PAD_HEIGHT_MULTIPLIER, width);
	prepare_refresh();
}

scrollable::~scrollable() {
	werase(m_pad);
	delwin(m_pad);
}

void scrollable::scroll_y(int delta) {
	if(m_scroll_amount == 0 && delta < 0)
		return;
	if(!(m_scroll_amount + m_screen_height > m_content_height && delta < 0)) {
		if(m_content_height < m_screen_height)
			return;
		if(m_scroll_amount + delta > m_content_height - m_screen_height)
			return;
	}
	m_scroll_amount += delta;
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

void scrollable::del_line(int x, int y) {
	wmove(m_pad, y, x);
	wclrtoeol(m_pad);
	if(y == m_content_height + m_content_start_y)
		m_content_height = std::max(0, (int) m_content_height - 1);
	else if(y == m_content_start_y) {
		++m_content_start_y;
		m_content_height = std::max(0, (int) m_content_height - 1);
	}
}

void scrollable::clear() {
	werase(m_pad);
	m_content_height = 0;
	m_content_start_y = 0;
}

void scrollable::prepare_refresh() {
	static bool clear_scrollbar = false;

	uint32_t segments_start;
	uint32_t number_of_segments;

	if(m_content_height > m_screen_height) {
		number_of_segments = std::max((uint32_t) 1, m_screen_height * m_screen_height / m_content_height);
		const uint32_t internal_scroll_amount = std::clamp(m_scroll_amount, (uint32_t) 0, m_content_height - m_screen_height);
		segments_start =
				(m_screen_height - number_of_segments) * internal_scroll_amount /
						(m_content_height - m_screen_height);
		clear_scrollbar = true;
	}else {
		number_of_segments = 0;
		if(!clear_scrollbar)
			clear_scrollbar = true;
	}

	for(int i = 0; clear_scrollbar && i < m_screen_height * PAD_HEIGHT_MULTIPLIER; ++i)
		mvwaddch(m_pad, i, m_width - 1, ' ');

	clear_scrollbar = false;

	for(int i = 0; i < number_of_segments; ++i)
		mvwaddch(m_pad, segments_start + i + m_scroll_amount + m_content_start_y, m_width - 1, '|');

	pnoutrefresh(m_pad, m_scroll_amount + m_content_start_y, 0, m_y_start, m_x_start, m_y_start + m_screen_height,
			m_x_start + m_width);
}

uint32_t scrollable::get_width() const {
	return m_width;
}