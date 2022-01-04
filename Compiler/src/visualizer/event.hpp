#pragma once

#include "layouting.hpp"
#include "logger.hpp"
#include "windows.hpp"
#include <sstream>

class event {
public:
	explicit event(unsigned int position);

	virtual void exec() const = 0;
	virtual void undo() const = 0;

	virtual CH::str to_string() const = 0;

	unsigned int getPosition() const;

protected:
	unsigned int m_position;

	template <typename Callback>
	void exec_on_cat_at_pos(unsigned int pos, Callback callback) const;
};

class event_with_data : public event {
public:
	event_with_data(unsigned int position, CH::str data);
	const CH::str& getData() const;

protected:
	CH::str m_data;
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

class message_event : public event_with_data {
public:
	using event_with_data::event_with_data;
	explicit message_event(const CH::str& data);
	void exec() const override;
	void undo() const override;
	CH::str to_string() const override;
};