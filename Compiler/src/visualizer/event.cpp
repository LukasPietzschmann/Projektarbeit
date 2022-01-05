#include "event.hpp"

long event::m_next_id = 0;

event::event(unsigned int position) : m_position(position), m_id(m_next_id++) {}

unsigned int event::getPosition() const {
	return m_position;
}

template <typename Callback>
void event::exec_on_cat_at_pos(unsigned int pos, Callback callback) const {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [&pos](const archive& c) {
		return c.get_pos_in_src() == pos;
	});
	assert(it != arch_windows.end());
	callback(*it);
}

Expr event_with_data::getData() const {
	return m_data;
}

event_with_data::event_with_data(unsigned int position, const Expr& data) :
		event(position), m_data(data) {}

void add_cons_event::exec() const {
	exec_on_cat_at_pos(m_position, [this](archive& c) {
		log(to_string());
		c.add_cons(m_id, m_data);
	});
}

void add_cons_event::undo() const {
	exec_on_cat_at_pos(m_position, [this](archive& c) {
		unlog();
		c.remove_cons_with_id(m_id);
	});
}

CH::str add_cons_event::to_string() const {
	std::stringstream ss;
	ss << "Added cons. expr. at pos " << m_position;
	return CH::str(ss.str());
}

void add_comp_event::exec() const {
	exec_on_cat_at_pos(m_position, [this](archive& c) {
		log(to_string());
		c.add_comp(m_id, m_data);
	});
}

void add_comp_event::undo() const {
	exec_on_cat_at_pos(m_position, [this](archive& c) {
		unlog();
		c.remove_comp_with_id(m_id);
	});
}

CH::str add_comp_event::to_string() const {
	std::stringstream ss;
	ss << "Added comp. expr. at pos " << m_position;
	return CH::str(ss.str());
}

void create_archive_event::exec() const {
	log(to_string());
	layouter::the().register_new_cat(arch_windows.emplace_back(m_position));
}

void create_archive_event::undo() const {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [this](const archive& c) {
		return c.get_pos_in_src() == m_position;
	});

	assert(it != arch_windows.end());
	unlog();
	layouter::the().unregister_cat(*it);
	arch_windows.erase(it);
}

CH::str create_archive_event::to_string() const {
	std::stringstream ss;
	ss << "Created arch. at Pos. " << m_position;
	return CH::str(ss.str());
}

void message_event::exec() const {
	log(to_string());
}

void message_event::undo() const {
	unlog();
}

CH::str message_event::to_string() const {
	return m_message;
}

message_event::message_event(const CH::str& data) : event(0), m_message(data) {}