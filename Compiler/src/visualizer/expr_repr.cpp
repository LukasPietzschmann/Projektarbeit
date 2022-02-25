#include "expr_repr.hpp"

expr_repr::expr_repr(const Expr& expr, int flags) :
		expr(expr), flags(flags) {
	if(expr(beg_) == expr(end_))
		this->flags |= f_is_prototype;
	else
		this->flags &= ~f_is_prototype;
}

//FIXME: nicht gerade ideal hier currpart_pos zu setzten
//		1: as_string kann dann nicht mehr const sein
//		2: es fühlt sich einfach nicht richtig an :|
CH::str expr_repr::as_string() {
	const auto& id_or_error = oper_store::the().get_id_from_oper(expr(oper_));
	const CH::str& id_str = CH::str(std::to_string(*id_or_error));
	const CH::str& id_prefix = id_or_error.has_value() ? (id_str + ": ") : "?: ";

	CH::str result = id_prefix;

	if(flags & f_is_prototype) {
		currpart_pos = *result;
		result += expr(to_str_);
	}else if(flags & f_is_comp) {
		result += get_scanned_str_for_expr(expr);
		currpart_pos = *result;
	}else {
		result += get_scanned_str_for_expr(expr);
		currpart_pos = *result;
		result += expr(to_str_from_currpart_);
	}
	const auto& pos = result(CH::searchA([](const char c) { return c == '\n'; }));
	if(pos != 0 * CH::A ) {
		result = result(pos | pos + 1, "\\n");
		++currpart_pos;
	}

	if(*result > REPLACE_WITH_ID_THRESHOLD && id_or_error.has_value() && flags & f_is_prototype)
		result = truncate_string_to_length(result, REPLACE_WITH_ID_THRESHOLD);

	return result;
}