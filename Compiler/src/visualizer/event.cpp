#include "event.hpp"

#include <utility>

event::event(unsigned int position) : m_position(position) {}

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

const std::string& a_event_with_data::getData() const {
	return m_data;
}

a_event_with_data::a_event_with_data(unsigned int position, std::string data) :
		event(position), m_data(std::move(data)) {}

void add_cons_event::exec() const {
	exec_on_cat_at_pos(m_position, [*this](archive& c) {
		c.add_cons(m_data);
	});
	log(to_string());
}

void add_cons_event::undo() const {
	exec_on_cat_at_pos(m_position, [*this](archive& c) {
		c.remove_cons(m_data);
	});
	unlog();
}

std::string add_cons_event::to_string() const {
	return "Added cons. expr. " + m_data + " at pos " + std::to_string(m_position);
}

void add_comp_event::exec() const {
	exec_on_cat_at_pos(m_position, [*this](archive& c) {
		c.add_comp(m_data);
	});
	log(to_string());
}

void add_comp_event::undo() const {
	exec_on_cat_at_pos(m_position, [*this](archive& c) {
		c.remove_comp(m_data);
	});
	unlog();
}

std::string add_comp_event::to_string() const {
	return "Added comp. expr. " + m_data + " at pos " + std::to_string(m_position);
}

void create_archive_event::exec() const {
	layouter::the().register_new_cat(arch_windows.emplace_back(m_position));
	log(to_string());
}

void create_archive_event::undo() const {
	const auto& it = std::find_if(arch_windows.begin(), arch_windows.end(), [*this](const archive& c) {
		return c.get_pos_in_src() == m_position;
	});

	assert(it != arch_windows.end());
	layouter::the().unregister_cat(*it);
	arch_windows.erase(it);
	unlog();
}

std::string create_archive_event::to_string() const {
	return "Created Cat at Pos. " + std::to_string(m_position);
}