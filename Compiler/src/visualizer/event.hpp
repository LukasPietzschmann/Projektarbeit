#pragma once

#include "layouting.hpp"
#include "logger.hpp"
#include "windows.hpp"
#include <string>

class event {
public:
	explicit event(unsigned int position);

	virtual void exec() const = 0;
	virtual void undo() const = 0;

	virtual std::string to_string() const = 0;

	unsigned int getPosition() const;

protected:
	unsigned int m_position;

	template <typename Callback>
	void exec_on_cat_at_pos(unsigned int pos, Callback callback) const;
};

class a_event_with_data : public event {
public:
	a_event_with_data(unsigned int position, std::string data);
	const std::string& getData() const;

protected:
	std::string m_data;
};

class add_cons_event : public a_event_with_data {
public:
	using a_event_with_data::a_event_with_data;
	void exec() const override;
	void undo() const override;
	std::string to_string() const override;
};

class add_comp_event : public a_event_with_data {
public:
	using a_event_with_data::a_event_with_data;
	void exec() const override;
	void undo() const override;
	std::string to_string() const override;
};

class create_archive_event : public event {
public:
	using event::event;
	void exec() const override;
	void undo() const override;
	std::string to_string() const override;
};