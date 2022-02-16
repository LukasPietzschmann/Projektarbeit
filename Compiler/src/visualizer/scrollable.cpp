#include "scrollable.hpp"

scrollable::scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start) : window_like<WINDOW>(
		newpad(height * PAD_HEIGHT_MULTIPLIER, width)), m_width(width), m_screen_height(height), m_x_start(x_start),
		m_y_start(y_start) {
	// In einem Konstruktor ist der virtuelle Aufrufmechanismus deaktiviert,
	// da das Überschreiben von abgeleiteten Klassen noch nicht stattgefunden hat.
	// Also sollten hier in der Regel keine virtuellen Methoden aufgerufen werden!
	// Durch die explizite Angabe des Scopes ist der Aufruf hier aber okay!
	scrollable::prepare_refresh();
}

scrollable::~scrollable() {
	werase(m_underlying_window);
	delwin(m_underlying_window);
}

void scrollable::scroll_y(int delta) {
	const auto& is_allowed_to_scroll = [&]() {
		if(delta == 0)
			return false;
		if(m_scroll_amount == 0 && delta < 0)
			return false;
		if(m_scroll_amount + m_screen_height > m_content_height) {
			if(delta < 0)
				return true;
			return false;
		}
		if(m_content_height < m_screen_height)
			return false;
		if(m_scroll_amount + m_screen_height + delta > m_content_height)
			return false;
		return true;
	};

	if(!is_allowed_to_scroll())
		return;

	m_scroll_amount += delta;
	prepare_refresh();
}

void scrollable::add_n_str(const CH::str& string, int x, int y) {
	mvwaddnstr(m_underlying_window, y, x, &string.elems[0], *string);
	if(y < m_content_start_y) {
		const int diff = m_content_start_y - y;
		m_content_start_y -= diff;
		m_content_height += diff;
	}else if(y >= m_content_height + m_content_start_y)
		m_content_height = y - m_content_start_y;
}

void scrollable::del_line(int x, int y) {
	wmove(m_underlying_window, y, x);
	wclrtoeol(m_underlying_window);
	if(y == m_content_height + m_content_start_y)
		m_content_height = std::max(0, (int) m_content_height - 1);
	else if(y == m_content_start_y) {
		++m_content_start_y;
		m_content_height = std::max(0, (int) m_content_height - 1);
	}
}

void scrollable::clear() {
	werase(m_underlying_window);
	m_content_height = 0;
	m_content_start_y = 0;
}

void scrollable::prepare_refresh() const {
	static bool clear_scrollbar = false;

	uint32_t segments_start;
	uint32_t number_of_segments;

	if(m_content_height > m_screen_height) {
		number_of_segments = std::max((uint32_t) 1, m_screen_height * m_screen_height / m_content_height);
		const uint32_t internal_scroll_amount = std::clamp(m_scroll_amount, (uint32_t) 0,
				m_content_height - m_screen_height);
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
		mvwaddch(m_underlying_window, i, m_width - 1, ' ');

	clear_scrollbar = false;

	for(int i = 0; i < number_of_segments; ++i)
		mvwaddch(m_underlying_window, segments_start + i + m_scroll_amount + m_content_start_y, m_width - 1, '|');

	pnoutrefresh(m_underlying_window, m_scroll_amount + m_content_start_y, 0, m_y_start, m_x_start,
			m_y_start + m_screen_height,
			m_x_start + m_width);
}

uint32_t scrollable::get_width() const {
	return m_width;
}