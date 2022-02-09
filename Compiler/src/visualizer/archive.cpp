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
	for(int i = 0; i < m_height; ++i) {
		mvsaddch(main_viewport, i + m_y_start, m_x_start, '|');
		mvsaddch(main_viewport, i + m_y_start, m_width + m_x_start, '|');
	}

	for(int i = 0; i <= m_width; ++i) {
		mvsaddch(main_viewport, m_y_start, i + m_x_start, '-');
		mvsaddch(main_viewport, m_height + m_y_start - 1, i + m_x_start, '-');
	}

	auto comp_it = m_comp.begin();
	auto cons_it = m_cons.begin();
	for(int i = 1; i < m_height - 1; ++i) {
		mvsaddch(main_viewport, i + m_y_start, m_divider_x_pos + m_x_start, '|');

		if(comp_it != m_comp.end()) {
			const auto& elem = comp_it->second;
			const auto& string = elem.as_string();
			if(elem.is_highlighted)
				wattron(**main_viewport, HIGHLIGHT_EXPR_ATTR);
			if(elem.is_ambiguous)
				wattron(**main_viewport, COLOR_PAIR(AMBIGUOUS_COLOR_PAIR));
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos + 1 + m_x_start, string);
			if(elem.is_highlighted)
				wattroff(**main_viewport, HIGHLIGHT_EXPR_ATTR);
			if(elem.is_ambiguous)
				wattroff(**main_viewport, COLOR_PAIR(AMBIGUOUS_COLOR_PAIR));
			++comp_it;
		}

		if(cons_it != m_cons.end()) {
			const auto& elem = cons_it->second;
			const auto& string = elem.as_string();
			if(elem.is_highlighted)
				wattron(**main_viewport, HIGHLIGHT_EXPR_ATTR);
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos - *string + m_x_start, string);
			if(elem.is_highlighted)
				wattroff(**main_viewport, HIGHLIGHT_EXPR_ATTR);
			++cons_it;
		}
	}
	const CH::str pos_in_src_str(std::to_string(m_pos_in_src));
	int pos = main_viewport_center + (-src_str_center + m_pos_in_src) - m_x_start;

	if(pos >= m_width)
		pos -= (pos - m_width) + *pos_in_src_str;

	mvsaddstr(main_viewport, m_y_start, pos + m_x_start, pos_in_src_str);
}

uint32_t archive::get_pos_in_src() const {
	return m_pos_in_src;
}

void archive::add_cons(long id, const Expr& cons) {
	bool is_proto = cons(beg_) == cons(end_);
	m_cons.try_emplace(id, cons, false, is_proto);
	invalidate();
}

void archive::add_comp(long id, const Expr& comp) {
	bool is_comp_ambiguous = false;
	for(auto&[id, element]: m_comp) {
		if(element.expr(end_) != comp(end_))
			continue;
		element.is_ambiguous = true;
		is_comp_ambiguous = true;
	}
	m_comp.try_emplace(id, comp, true, false, false, is_comp_ambiguous);
	invalidate();
}

bool archive::remove_cons_with_id(long id) {
	if(m_cons.erase(id) == 0)
		return false;

	invalidate();
	return true;
}

bool archive::remove_comp_with_id(long id) {
	const auto& it = m_comp.find(id);
	if(it == m_comp.end())
		return false;

	if(it->second.is_ambiguous) {
		for(auto &[e_id, elem]: m_comp) {
			if(e_id == id)
				continue;
			if(elem.expr(end_) != it->second.expr(end_))
				continue;
			elem.is_ambiguous = false;
		}
	}

	m_comp.erase(it);
	invalidate();
	return true;
}

bool archive::set_expr_active(const Expr& expr) {
	const auto& do_in = [&](map<long, archive::archive_element>& elements) {
		for(auto&[id, element]: elements) {
			if(element.expr != expr)
				continue;
			element.is_highlighted = true;
			invalidate();
			return true;
		}
		return false;
	};

	return do_in(m_comp) || do_in(m_cons);
}

bool archive::set_expr_inactive(const Expr& expr) {
	const auto& do_in = [&](map<long, archive::archive_element>& elements) {
		for(auto&[id, element]: elements) {
			if(element.expr != expr)
				continue;
			element.is_highlighted = false;
			invalidate();
			return true;
		}
		return false;
	};

	return do_in(m_comp) || do_in(m_cons);
}

void archive::set_y_start(uint32_t y_start) {
	if(m_y_start == y_start)
		return;
	m_y_start = y_start;
}

void archive::set_x_start(uint32_t x_start) {
	if(m_x_start == x_start)
		return;
	m_x_start = x_start;
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

uint32_t archive::get_divider_x_pos() const {
	return m_divider_x_pos;
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
	static const auto longest_str_len = [](const std::map<long, archive_element>& elements) {
		unsigned long max = 0;
		for(const auto&[id, elem]: elements) {
			const auto& string = elem.as_string();
			if(*string > max)
				max = *string;
		}
		return max;
	};

	const auto cons_len = std::max((unsigned long) 1, longest_str_len(m_cons));
	const auto comp_len = std::max((unsigned long) 1, longest_str_len(m_comp));

	uint32_t new_width = comp_len + 5 + cons_len + 5;
	m_divider_x_pos = cons_len + 5;

	if(new_width % 2 == 0)
		++new_width;

	m_height = std::max(m_comp.size(), m_cons.size()) + 2;;
	m_width = new_width;
	for(const archive_change_listener* listener: m_listeners)
		listener->notify_dimensions_changed(*this);
}

bool archive::operator==(const archive& other) const {
	return this == &other;
}

CH::str archive::archive_element::as_string() const {
	const auto& id_or_error = oper_store::the().get_id_from_oper(expr(oper_));
	const CH::str& id_str = CH::str(std::to_string(*id_or_error));
	const CH::str& id_prefix = id_or_error.has_value() ? (id_str + ": ") : "?: ";

	CH::str result;

	if(is_prototyp)
		result += expr(to_str_);
	else if(!is_comp)
		result += get_scanned_str_for_expr(expr) + expr(to_str_from_currpart_);
	else
		result += get_scanned_str_for_expr(expr);

	if(*result > REPLACE_WITH_ID_THRESHOLD && id_or_error.has_value())
		result = "ID: " + id_str;

	return result;
}