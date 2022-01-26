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

	void push_back(const Expr& expr);
	bool pop_front();

private:
	expr_queue() = default;
	int m_y_begin {0};
	int m_y_end {0};
};