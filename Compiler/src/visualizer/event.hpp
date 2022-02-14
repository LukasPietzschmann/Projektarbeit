#pragma once

#include <algorithm>
#include <initializer_list>
#include <functional>
#include <sstream>
#include <memory>
#include <vector>

#include "../flxc/data.h"
#include "layouting.hpp"
#include "expr_queue.hpp"
#include "oper_store.hpp"

class event {
public:
	enum event_exec_result {
		did_something,
		did_nothing
	};

	explicit event(unsigned int position);
	virtual ~event() = default;

	virtual event_exec_result exec() = 0;
	virtual event_exec_result undo() = 0;

protected:
	long m_id;
	unsigned int m_position;

	template <typename Callback>
	void exec_on_archive_at_pos(unsigned int pos, Callback callback) const;

private:
	static long m_next_id;
};

class event_with_data : public event {
public:
	event_with_data(unsigned int position, const Expr& data);

protected:
	Expr m_data;
};

class add_cons_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	event_exec_result exec() override;
	event_exec_result undo() override;
};

class add_comp_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	event_exec_result exec() override;
	event_exec_result undo() override;
};

class create_archive_event : public event {
public:
	using event::event;
	event_exec_result exec() override;
	event_exec_result undo() override;
};

class expr_gets_used_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	event_exec_result exec() override;
	event_exec_result undo() override;
};

class expr_no_longer_gets_used_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	event_exec_result exec() override;
	event_exec_result undo() override;
};

class add_expr_to_queue : public event_with_data {
public:
	using event_with_data::event_with_data;
	explicit add_expr_to_queue(const Expr& expr, bool is_comp);
	event_exec_result exec() override;
	event_exec_result undo() override;

private:
	bool m_is_comp;
};

class remove_expr_from_queue : public event_with_data {
public:
	using event_with_data::event_with_data;
	explicit remove_expr_from_queue(const Expr& expr, bool is_comp);
	event_exec_result exec() override;
	event_exec_result undo() override;

private:
	bool m_is_comp;
};

class event_group : public event {
public:
	using event::event;
	event_group(std::initializer_list<event*>);
	event_exec_result exec() override;
	event_exec_result undo() override;
private:
	std::vector<std::unique_ptr<event>> m_events;
};