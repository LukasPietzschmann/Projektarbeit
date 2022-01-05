#pragma once

#include "layouting.hpp"
#include "logger.hpp"
#include "windows.hpp"
#include "../flxc/data.h"
#include <initializer_list>
#include <sstream>
#include <memory>
#include <vector>

class event {
public:
	explicit event(unsigned int position);

	virtual void exec() const = 0;
	virtual void undo() const = 0;

	virtual CH::str to_string() const = 0;

	unsigned int getPosition() const;

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
	Expr getData() const;

protected:
	Expr m_data;
};

class add_cons_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	void exec() const override;
	void undo() const override;
	CH::str to_string() const override;
};

class add_comp_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	void exec() const override;
	void undo() const override;
	CH::str to_string() const override;
};

class create_archive_event : public event {
public:
	using event::event;
	void exec() const override;
	void undo() const override;
	CH::str to_string() const override;
};

class expr_gets_used_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	void exec() const override;
	void undo() const override;
	str to_string() const override;
};

class expr_no_longer_gets_used_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	void exec() const override;
	void undo() const override;
	str to_string() const override;
};

class message_event : public event {
public:
	using event::event;
	explicit message_event(const CH::str& message);
	void exec() const override;
	void undo() const override;
	CH::str to_string() const override;
private:
	CH::str m_message;
};

class event_group : public event {
public:
	using event::event;
	explicit event_group(std::initializer_list<event*>);
	void exec() const override;
	void undo() const override;
	str to_string() const override;
private:
	std::vector<std::unique_ptr<event>> m_events;
};