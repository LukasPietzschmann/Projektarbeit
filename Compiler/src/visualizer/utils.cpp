#include "utils.hpp"

void center_text_hor(WINDOW* window, const CH::str& str, uint32_t y_coordinate) {
	assert(window != nullptr);
	uint32_t x_coordinate = (getmaxx(window) / 2) - (*str / 2);
	mvwaddnstr(window, y_coordinate, x_coordinate, &str.elems[0], *str);
	wnoutrefresh(window);
}

/*CH::str expr_to_str(const Expr& expr) {
	const auto& id_or_error = oper_store::the().get_id_from_oper(expr(oper_));
	const CH::str& id_str = CH::str(std::to_string(*id_or_error));
	const CH::str& id_prefix = id_or_error.has_value() ? (id_str + ": ") : "?: ";

	CH::str result;

	if(expr(beg_) == expr(end_))
		result = id_prefix + " " + expr(to_str_);
	else
		result = id_prefix + " " + get_scanned_str_for_expr(expr) + expr(to_str_from_currpart_);

	if(*result > REPLACE_WITH_ID_THRESHOLD && id_or_error.has_value())
		result = "ID: " + id_str;

	return result;
}*/