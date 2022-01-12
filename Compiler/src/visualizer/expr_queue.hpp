#pragma once

#include <ncurses.h>
#include "../flxc/data.h"
#include "constants.hpp"
#include "coordinates.hpp"
#include "windows.hpp"

class expr_queue {
public:
	static expr_queue& the() {
		static expr_queue instance;
		return instance;
	}

	expr_queue(const expr_queue&) = delete;
	expr_queue(expr_queue&&) noexcept = default;
	expr_queue& operator=(const expr_queue&) = delete;
	expr_queue& operator=(expr_queue&&) = default;

	void push(const Expr& expr);
	bool pop();

	void scroll_y(int delta);
private:
	expr_queue() = default;
	uint32_t m_size {0};
	int m_scroll_y {0};
};