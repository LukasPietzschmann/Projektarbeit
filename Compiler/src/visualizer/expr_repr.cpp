#include "expr_repr.hpp"

expr_repr::expr_repr(const Expr& expr, int flags) :
		expr(expr), flags(flags) {
	if(expr(beg_) == expr(end_))
		flags |= f_is_prototype;
	else
		flags &= ~f_is_prototype;
}

CH::str expr_repr::as_string() const {
	const auto& id_or_error = oper_store::the().get_id_from_oper(expr(oper_));
	const CH::str& id_str = CH::str(std::to_string(*id_or_error));
	const CH::str& id_prefix = id_or_error.has_value() ? (id_str + ": ") : "?: ";

	CH::str result;

	if(flags & f_is_prototype)
		result += expr(to_str_);
	else if(!(flags & f_is_comp))
		result += get_scanned_str_for_expr(expr) + expr(to_str_from_currpart_);
	else
		result += get_scanned_str_for_expr(expr);
	result = id_prefix + result;

	if(*result > REPLACE_WITH_ID_THRESHOLD && id_or_error.has_value())
		result = "ID: " + id_str;

	return result;
}