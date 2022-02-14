#include "expr_repr.hpp"

expr_repr::expr_repr(const Expr& expr, bool is_comp, bool is_prototyp, bool is_highlighted, bool is_ambiguous) :
		expr(expr), is_comp(is_comp), is_highlighted(is_highlighted), is_ambiguous(is_ambiguous),
		is_prototyp(is_prototyp) {}

CH::str expr_repr::as_string() const {
	return "tada";
	const auto& id_or_error = oper_store::the().get_id_from_oper(expr(oper_));
	const CH::str& id_str = CH::str(std::to_string(*id_or_error));
	const CH::str& id_prefix = id_or_error.has_value() ? (id_str + ": ") : "?: ";

	CH::str result;

	if(is_prototyp)
		result += expr(to_str_);
	else if(!is_comp)
		result += get_scanned_str_for_expr(expr) + expr(to_str_from_currpart_);
	else
		result += get_scanned_str_for_expr(expr);

	if(*result > REPLACE_WITH_ID_THRESHOLD && id_or_error.has_value())
		result = "ID: " + id_str;

	return result;
}