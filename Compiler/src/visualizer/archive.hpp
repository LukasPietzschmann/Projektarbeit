#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <ncurses.h>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <seq.ch>

#include "../flxc/data.h"
#include "../flxc/scanner.h"
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

	explicit archive(uint32_t pos_in_src) : m_pos_in_src(pos_in_src) {
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

	void add_cons(long id, const Expr& cons);
	void add_comp(long id, const Expr& comp);
	bool remove_cons_with_id(long id);
	bool remove_comp_with_id(long id);

	bool set_expr_active(const Expr& expr);
	bool set_expr_inactive(const Expr& elements);

	void set_y_start(uint32_t y_start);
	void set_x_start(uint32_t x_start);

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
	struct archive_element {
		explicit archive_element(const Expr& expr, bool is_highlighted = false) :
				expr(expr), is_highlighted(is_highlighted) {};

		Expr expr;
		bool is_highlighted;

		CH::str as_string() const;
	};

	WINDOW* m_window {nullptr};
	uint32_t m_width {0};
	uint32_t m_height {0};
	uint32_t m_y_start {0};
	uint32_t m_x_start {0};
	uint32_t m_pos_in_src;

	bool m_dirty {true};

	std::map<long, archive_element> m_cons {}, m_comp {};

	t_id m_id {-1};

	std::vector<archive_change_listener*> m_listeners {};

	void invalidate();
};