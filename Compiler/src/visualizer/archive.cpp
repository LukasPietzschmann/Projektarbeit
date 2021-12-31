#include "archive.hpp"

bool archive::intersects_with(const archive& other) const {
	return intersects_with(rect(other));
}

bool archive::intersects_with(const archive::rect& other) const {
	if(m_x_start > other.x + other.width || other.x > m_x_start + m_width)
		return false;
	if(m_y_start > other.y + other.height || other.y > m_y_start + m_height)
		return false;
	return true;
}

void archive::render() {
	if(m_window == nullptr) {
		m_window = derwin(stdscr, m_height, m_width, m_y_start + 1, m_x_start);
		assert(m_window != nullptr);
	}else {
		wclear(m_window);
		wresize(m_window, m_height, m_width);
		//TODO wieso get mvderwin(m_window, m_y_start, m_x_start); nicht?
		mvwin(m_window, getbegy(m_window), m_x_start);
	}

	box(m_window, ACS_VLINE, ACS_HLINE);

	uint32_t center = m_width / 2;
	for(int i = 1; i < m_height - 1; ++i) {
		mvwaddch(m_window, i, center, '|');

		if(i - 1 < m_comp.size()) {
			const auto& elem = m_comp[i - 1];
			mvwaddnstr(m_window, i, center - *elem, &elem.elems[0], *elem);
		}

		if(i - 1 < m_cons.size()) {
			const auto& elem = m_cons[i - 1];
			mvwaddnstr(m_window, i, center + 1, &elem.elems[0], *elem);
		}
	}
	center_text_hor(m_window, CH::str(std::to_string(m_pos_in_src)), 0);

	wnoutrefresh(m_window);
}

uint32_t archive::get_pos_in_src() const {
	return m_pos_in_src;
}

bool archive::add_cons(const CH::str& cons) {
	if(std::find(archive::m_cons.begin(), archive::m_cons.end(), cons) != archive::m_cons.end())
		return false;
	archive::m_cons.push_back(cons);
	invalidate();
	return true;
}

bool archive::add_comp(const CH::str& comp) {
	if(std::find(archive::m_comp.begin(), archive::m_comp.end(), comp) != archive::m_comp.end())
		return false;
	archive::m_comp.push_back(comp);
	invalidate();
	return true;
}

bool archive::remove_cons(const CH::str& cons) {
	if(const auto& it = std::find(archive::m_cons.begin(), archive::m_cons.end(), cons); it != archive::m_cons.end()) {
		archive::m_cons.erase(it);
		invalidate();
		return true;
	}
	return false;
}

bool archive::remove_comp(const CH::str& comp) {
	if(const auto& it = std::find(archive::m_comp.begin(), archive::m_comp.end(), comp); it != archive::m_comp.end()) {
		archive::m_comp.erase(it);
		invalidate();
		return true;
	}
	return false;
}

void archive::set_y_start(uint32_t yStart) {
	m_y_start = yStart;
}

void archive::set_x_start(uint32_t xStart) {
	m_x_start = xStart;
}

uint32_t archive::get_width() const {
	return m_width;
}

uint32_t archive::get_height() const {
	return m_height;
}

uint32_t archive::get_y_start() const {
	return m_y_start;
}

uint32_t archive::get_x_start() const {
	return m_x_start;
}

archive::t_id archive::id() {
	static t_id next_id = 0;

	if(m_id == -1)
		m_id = next_id++;

	return m_id;
}

void archive::register_as_listener(archive_change_listener* listener) {
	m_listeners.push_back(listener);
	listener->notify_dimensions_changed(*this);
}

void archive::unregister_as_listener(archive_change_listener* listener) {
	const auto& it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if(it == m_listeners.end())
		return;
	m_listeners.erase(it);
}

void archive::invalidate() {
	static const auto longest_str_len = [](const std::vector<CH::str>& strings) {
		unsigned long max = 0;
		for(const auto& str: strings) {
			if(*str > max)
				max = *str;
		}
		return max;
	};

	uint32_t new_height, new_width;

	const auto cons_len = longest_str_len(m_cons);
	const auto comp_len = longest_str_len(m_comp);

	if(cons_len > comp_len)
		new_width = cons_len * 2 + 11;
	else
		new_width = comp_len * 2 + 11;

	if(new_width % 2 == 0)
		++new_width;
	new_height = std::max(m_comp.size(), m_cons.size()) + 2;

	if(m_height != new_height || m_width != new_width) {
		m_height = new_height;
		m_width = new_width;
		for(const archive_change_listener* listener: m_listeners)
			listener->notify_dimensions_changed(*this);
	}
}
bool archive::operator==(const archive& other) const {
	return this == &other;
}