#include "event.hpp"

long event::m_next_id = 0;

event::event(unsigned int position) : m_position(position), m_id(m_next_id++) {}

unsigned int event::getPosition() const {
	return m_position;
}

template <typename Callback>
void event::exec_on_archive_at_pos(unsigned int pos, Callback callback) const {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [&pos](const archive& c) {
		return c.get_pos_in_src() == pos;
	});
	if(it == arch_windows.end())
		return;
	callback(*it);
}

Expr event_with_data::getData() const {
	return m_data;
}

event_with_data::event_with_data(unsigned int position, const Expr& data) :
		event(position), m_data(data) {}

event::event_exec_result add_cons_event::exec() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		log(to_string());
		c.add_cons(m_id, m_data);
	});
	return did_something;
}

event::event_exec_result add_cons_event::undo() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		unlog();
		c.remove_cons_with_id(m_id);
	});
	return did_something;
}

CH::str add_cons_event::to_string() const {
	std::stringstream ss;
	ss << "Added cons. expr. at pos " << m_position;
	return CH::str(ss.str());
}

event::event_exec_result add_comp_event::exec() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		log(to_string());
		c.add_comp(m_id, m_data);
	});
	return did_something;
}

event::event_exec_result add_comp_event::undo() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		unlog();
		c.remove_comp_with_id(m_id);
	});
	return did_something;
}

CH::str add_comp_event::to_string() const {
	std::stringstream ss;
	ss << "Added comp. expr. at pos " << m_position;
	return CH::str(ss.str());
}

event::event_exec_result create_archive_event::exec() {
	log(to_string());
	layouter::the().register_new_cat(arch_windows.emplace_back(m_position));
	return did_something;
}

event::event_exec_result create_archive_event::undo() {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [this](const archive& c) {
		return c.get_pos_in_src() == m_position;
	});

	assert(it != arch_windows.end());
	unlog();
	layouter::the().unregister_cat(*it);
	arch_windows.erase(it);
	return did_something;
}

CH::str create_archive_event::to_string() const {
	std::stringstream ss;
	ss << "Created arch. at Pos. " << m_position;
	return CH::str(ss.str());
}

event::event_exec_result expr_gets_used_event::exec() {
	bool res;
	exec_on_archive_at_pos(m_position, [this, &res](archive& a) {
		res = a.set_expr_active(m_data);
	});
	return res ? did_something : did_nothing;
}

event::event_exec_result expr_gets_used_event::undo() {
	bool res;
	exec_on_archive_at_pos(m_position, [this, &res](archive& a) {
		res = a.set_expr_inactive(m_data);
	});
	return res ? did_something : did_nothing;
}

CH::str expr_gets_used_event::to_string() const {
	return {""};
}

event::event_exec_result expr_no_longer_gets_used_event::exec() {
	bool res;
	exec_on_archive_at_pos(m_position, [this, &res](archive& a) {
		res = a.set_expr_inactive(m_data);
	});
	return res ? did_something : did_nothing;
}

event::event_exec_result expr_no_longer_gets_used_event::undo() {
	bool res;
	exec_on_archive_at_pos(m_position, [this, &res](archive& a) {
		res = a.set_expr_active(m_data);
	});
	return res ? did_something : did_nothing;
}

CH::str expr_no_longer_gets_used_event::to_string() const {
	return {""};
}

event::event_exec_result message_event::exec() {
	log(to_string());
	return did_something;
}

event::event_exec_result message_event::undo() {
	unlog();
	return did_something;
}

CH::str message_event::to_string() const {
	return m_message;
}

message_event::message_event(const CH::str& data) : event(0), m_message(data) {}

event_group::event_group(std::initializer_list<event*> events) : event(0) {
	m_events.reserve(events.size());
	for(auto* e: events)
		m_events.push_back(std::unique_ptr<event>(e));
}

event::event_exec_result event_group::exec() {
	bool is_one_false = false;
	for(const auto& event: m_events)
		is_one_false = is_one_false || event->exec() == did_nothing;
	return is_one_false ? did_nothing : did_something;
}

event::event_exec_result event_group::undo() {
	bool is_one_false = false;
	for(const auto& event: m_events)
		is_one_false = is_one_false || event->undo() == did_nothing;
	return is_one_false ? did_nothing : did_something;
}

str event_group::to_string() const {
	return {""};
}