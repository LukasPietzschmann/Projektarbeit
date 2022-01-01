#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <ncurses.h>
#include <string>
#include <utility>
#include <vector>
#include <seq.ch>

#include "constants.hpp"
#include "archive_change_listener.hpp"
#include "utils.hpp"
#include "windows.hpp"

class archive {
public:
	struct rect {
		rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) : x(x), y(y), width(width), height(height) {}
		explicit rect(const archive& c) :
				x(c.get_x_start()), y(c.get_y_start()), width(c.get_width()), height(c.get_height()) {}

		uint32_t x, y, width, height;
	};

	explicit archive(uint32_t pos_in_src, std::vector<CH::str> cons = {}, std::vector<CH::str> comp = {}) :
			m_pos_in_src(pos_in_src), m_cons(std::move(cons)), m_comp(std::move(comp)) {
		invalidate();
	}

	~archive() {
		if(m_window != nullptr) {
			wclear(m_window);
			wrefresh(m_window);
			werase(m_window);
		}
	}

	bool operator==(const archive& other) const;

	bool intersects_with(const archive& other) const;
	bool intersects_with(const rect& other) const;
	void render();

	uint32_t get_pos_in_src() const;

	bool add_cons(const CH::str& cons);
	bool add_comp(const CH::str& comp);
	bool remove_cons(const CH::str& cons);
	bool remove_comp(const CH::str& comp);

	void set_y_start(uint32_t yStart);
	void set_x_start(uint32_t xStart);

	uint32_t get_width() const;
	uint32_t get_height() const;
	uint32_t get_y_start() const;
	uint32_t get_x_start() const;

	using t_id = long;
	t_id id();

	void register_as_listener(archive_change_listener* listener);
	void unregister_as_listener(archive_change_listener* listener);

	bool is_layouted {false};

private:
	WINDOW* m_window {nullptr};
	uint32_t m_width {0};
	uint32_t m_height {0};
	uint32_t m_y_start {0};
	uint32_t m_x_start {0};
	uint32_t m_pos_in_src;

	std::vector<CH::str> m_cons, m_comp;

	t_id m_id {-1};

	std::vector<archive_change_listener*> m_listeners {};

	void invalidate();
};