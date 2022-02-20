#include "event.hpp"

long event::m_next_id = 0;

event::event(unsigned int position) : m_position(position), m_id(m_next_id++) {}

template <typename Callback>
void event::exec_on_archive_at_pos(unsigned int pos, Callback callback) const {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [&pos](const archive& c) {
		return c.get_pos_in_src() == pos;
	});
	if(it == arch_windows.end())
		return;
	callback(*it);
}

event_with_data::event_with_data(unsigned int position, const Expr& data) :
		event(position), m_data(data) {}

event::event_exec_result add_cons_event::exec() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		c.add_cons(m_id, m_data);
	});
	return did_something;
}

event::event_exec_result add_cons_event::undo() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		c.remove_cons_with_id(m_id);
	});
	return did_something;
}

event::event_exec_result add_comp_event::exec() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		c.add_comp(m_id, m_data);
	});
	return did_something;
}

event::event_exec_result add_comp_event::undo() {
	exec_on_archive_at_pos(m_position, [this](archive& c) {
		c.remove_comp_with_id(m_id);
	});
	return did_something;
}

event::event_exec_result create_archive_event::exec() {
	arch_windows.emplace_back(m_position).register_as_listener(&layouter::the());
	return did_something;
}

event::event_exec_result create_archive_event::undo() {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [this](const archive& c) {
		return c.get_pos_in_src() == m_position;
	});

	assert(it != arch_windows.end());
	it->unregister_as_listener(&layouter::the());
	arch_windows.erase(it);
	return did_something;
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

add_expr_to_queue::add_expr_to_queue(const Expr& expr, bool is_comp) : event_with_data(0, expr), m_is_comp(is_comp) {}

event::event_exec_result add_expr_to_queue::exec() {
	if(!m_is_comp)
		oper_store::the().insert_if_prototyp(m_data);
	expr_queue::the().push_back(m_data, m_is_comp);
	return event::did_something;
}

event::event_exec_result add_expr_to_queue::undo() {
	return expr_queue::the().pop_back() ? event::did_something : event::did_nothing;
}

remove_expr_from_queue::remove_expr_from_queue(const Expr& expr, bool is_comp) :
		event_with_data(0, expr), m_is_comp(is_comp) {}

event::event_exec_result remove_expr_from_queue::exec() {
	return expr_queue::the().pop_front() ? event::did_something : event::did_nothing;
}

event::event_exec_result remove_expr_from_queue::undo() {
	expr_queue::the().push_front(m_data, m_is_comp);
	return event::did_something;
}

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