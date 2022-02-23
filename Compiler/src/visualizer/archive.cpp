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
	// s. layouter::notify_dimensions_changed()
	// if(!m_dirty_visuals && !m_dirty_dimensions)
	// 	return;

	for(int i = 0; i < m_height; ++i) {
		mvsaddstr(main_viewport, i + m_y_start, m_x_start, "|");
		mvsaddstr(main_viewport, i + m_y_start, m_width + m_x_start, "|");
	}

	for(int i = 0; i <= m_width; ++i) {
		mvsaddstr(main_viewport, m_y_start, i + m_x_start, "-");
		mvsaddstr(main_viewport, m_height + m_y_start - 1, i + m_x_start, "-");
	}

	auto comp_it = m_comp.begin();
	auto cons_it = m_cons.begin();
	for(int i = 1; i < m_height - 1; ++i) {
		mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos + m_x_start, "|");

		if(comp_it != m_comp.end()) {
			auto elem = comp_it->second;
			const auto& string = elem.as_string();
			if(elem.flags & expr_repr::f_is_highlighted)
				wattron(**main_viewport, A_HIGHLIGHT);
			if(elem.flags & expr_repr::f_is_ambiguous)
				wattron(**main_viewport, A_AMBIGUOUS);
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos + 1 + m_x_start, string(A | Z - elem.currpart_pos));
			wattron(**main_viewport, A_MUTED);
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos + 1 + m_x_start + elem.currpart_pos,
					string(A + elem.currpart_pos | Z));
			wattroff(**main_viewport, A_MUTED);
			if(elem.flags & expr_repr::f_is_highlighted)
				wattroff(**main_viewport, A_HIGHLIGHT);
			if(elem.flags & expr_repr::f_is_ambiguous)
				wattroff(**main_viewport, A_AMBIGUOUS);
			++comp_it;
		}

		if(cons_it != m_cons.end()) {
			auto elem = cons_it->second;
			const auto& string = elem.as_string();
			if(elem.flags & expr_repr::f_is_highlighted)
				wattron(**main_viewport, A_HIGHLIGHT);
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos - *string + m_x_start,
					string(A | Z - elem.currpart_pos));
			wattron(**main_viewport, A_MUTED);
			mvsaddstr(main_viewport, i + m_y_start, m_divider_x_pos - (*string - elem.currpart_pos) + m_x_start,
					string(A + elem.currpart_pos | Z));
			wattroff(**main_viewport, A_MUTED);
			if(elem.flags & expr_repr::f_is_highlighted)
				wattroff(**main_viewport, A_HIGHLIGHT);
			++cons_it;
		}
	}
	const CH::str pos_in_src_str(std::to_string(m_pos_in_src));
	int pos = main_viewport_horizontal_center + (-src_str_center + m_pos_in_src) - m_x_start;

	if(pos >= m_width)
		pos -= (pos - m_width) + *pos_in_src_str;

	mvsaddstr(main_viewport, m_y_start, pos + m_x_start, pos_in_src_str);

	m_dirty_visuals = false;
	m_dirty_dimensions = false;
}

uint32_t archive::get_pos_in_src() const {
	return m_pos_in_src;
}

void archive::add_cons(long id, const Expr& cons) {
	m_cons.try_emplace(id, cons, false);
	m_dirty_dimensions = true;
	invalidate();
}

void archive::add_comp(long id, const Expr& comp) {
	int flags = 0;
	for(auto&[_, element]: m_comp) {
		if(element.expr(end_) != comp(end_))
			continue;
		element.flags |= expr_repr::f_is_ambiguous;
		m_dirty_visuals = true;
		flags = expr_repr::f_is_ambiguous;
	}
	flags |= expr_repr::f_is_comp;
	m_comp.try_emplace(id, comp, flags);
	m_dirty_dimensions = true;
	invalidate();
}

bool archive::remove_cons_with_id(long id) {
	if(m_cons.erase(id) == 0)
		return false;

	m_dirty_dimensions = true;
	invalidate();
	return true;
}

bool archive::remove_comp_with_id(long id) {
	const auto& it = m_comp.find(id);
	if(it == m_comp.end())
		return false;

	int number_of_exprs_with_same_end = 0;
	expr_repr* repr_if_one_expr_with_same_end;
	if(it->second.flags & expr_repr::f_is_ambiguous) {
		for(auto &[e_id, elem]: m_comp) {
			if(e_id == id)
				continue;
			if(elem.expr(end_) != it->second.expr(end_))
				continue;
			assert(elem.flags & expr_repr::f_is_ambiguous);
			++number_of_exprs_with_same_end;
			repr_if_one_expr_with_same_end = &elem;
		}
		// Wenn weiterhin mehr als ein Ausdruck mit der selben Länge im Archiv existiert,
		// soll natürlich auch weiterhin das f_is_ambiguous Flags gesetzt bleiben!
		if(number_of_exprs_with_same_end == 1) {
			assert(repr_if_one_expr_with_same_end != nullptr);
			repr_if_one_expr_with_same_end->flags &= ~expr_repr::f_is_ambiguous;
			m_dirty_visuals = true;
		}
	}

	m_comp.erase(it);
	m_dirty_dimensions = true;
	invalidate();
	return true;
}

bool archive::set_expr_active(const Expr& expr) {
	const auto& do_in = [&](map<long, expr_repr>& elements) {
		for(auto&[id, element]: elements) {
			if(element.expr != expr)
				continue;
			element.flags |= expr_repr::f_is_highlighted;
			m_dirty_visuals = true;
			invalidate();
			return true;
		}
		return false;
	};

	return do_in(m_comp) || do_in(m_cons);
}

bool archive::set_expr_inactive(const Expr& expr) {
	const auto& do_in = [&](map<long, expr_repr>& elements) {
		for(auto&[id, element]: elements) {
			if(element.expr != expr)
				continue;
			element.flags &= ~expr_repr::f_is_highlighted;
			m_dirty_visuals = true;
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
	m_dirty_dimensions = true;
	m_y_start = y_start;
}

void archive::set_x_start(uint32_t x_start) {
	if(m_x_start == x_start)
		return;
	m_dirty_dimensions = true;
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
	static const auto longest_str_len = [](const std::map<long, expr_repr>& elements) {
		unsigned long max = 0;
		for(auto[id, elem]: elements) {
			const auto& string = elem.as_string();
			if(*string > max)
				max = *string;
		}
		return max;
	};

	if(m_dirty_dimensions) {
		const auto cons_len = longest_str_len(m_cons);
		const auto comp_len = longest_str_len(m_comp);

		m_divider_x_pos = cons_len + ARCHIVE_X_PADDING;

		m_width = comp_len + ARCHIVE_X_PADDING + cons_len + ARCHIVE_X_PADDING;
		m_height = std::max(m_comp.size(), m_cons.size()) + ARCHIVE_Y_PADDING;

		for(const archive_change_listener* listener: m_listeners)
			listener->notify_dimensions_changed(*this);
	}
	if(m_dirty_visuals) {
		for(const archive_change_listener* listener: m_listeners)
			listener->notify_visuals_changed(*this);
	}
}

bool archive::operator==(const archive& other) const {
	return this == &other;
}