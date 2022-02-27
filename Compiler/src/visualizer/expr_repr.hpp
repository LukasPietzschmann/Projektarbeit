#pragma once

#include "../flxc/data.h"
#include "../flxc/scanner.h"

#include "constants.hpp"
#include "oper_store.hpp"
#include "utils.hpp"

struct expr_repr {
	enum flags {
		f_is_comp = 1 << 0,
		f_is_highlighted = 1 << 1,
		f_is_ambiguous = 1 << 2,
		f_is_prototype = 1 << 3,
	};

	explicit expr_repr(const Expr& expr, int flags = 0);

	Expr expr;
	int flags;
	uint32_t currpart_pos {0};

	CH::str as_string(int string_length_threshold = REPLACE_WITH_ID_THRESHOLD);
};