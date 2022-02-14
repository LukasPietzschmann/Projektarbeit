#pragma once

#include "../flxc/data.h"
#include "../flxc/scanner.h"
#include "oper_store.hpp"

struct expr_repr {
	expr_repr(const Expr& expr, bool is_comp, bool is_prototyp = false, bool is_highlighted = false,
			bool is_ambiguous = false);

	Expr expr;
	bool is_comp;
	bool is_highlighted;
	bool is_ambiguous;
	bool is_prototyp;

	CH::str as_string() const;
};