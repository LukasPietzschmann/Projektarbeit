#pragma once

#include <ncurses.h>

#include "../flxc/data.h"
#include "../flxc/scanner.h"
#include "../libCH/seq.ch"

#include "constants.hpp"
#include "coordinates.hpp"
#include "expr_repr.hpp"
#include "oper_store.hpp"
#include "scrollable.hpp"
#include "utils.hpp"
#include "windows.hpp"

class expr_queue {
public:
	static expr_queue& the();

	expr_queue(const expr_queue&) = delete;
	expr_queue(expr_queue&&) noexcept = default;
	expr_queue& operator=(const expr_queue&) = delete;
	expr_queue& operator=(expr_queue&&) = default;

	void push_back(const Expr& expr, bool is_comp);
	bool pop_back();
	void push_front(const Expr& expr, bool is_comp);
	bool pop_front();

private:
	expr_queue() = default;
	int m_y_begin {0};
	int m_y_end {0};
};