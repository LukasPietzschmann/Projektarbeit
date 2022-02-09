#include "expr_queue.hpp"

void expr_queue::push_back(const Expr& expr) {
	CH::str expr_str = expr_to_str(expr);
	if(*expr_str >= QUEUE_WIDTH)
		expr_str = expr_str(A | A + (QUEUE_WIDTH - 2));
	mvsaddstr(queue_display, m_y_end++, 0, expr_str);
	queue_display->prepare_refresh();
}

bool expr_queue::pop_back() {
	if(m_y_begin == m_y_end)
		return false;
	queue_display->del_line(0, --m_y_end);
	queue_display->prepare_refresh();
	return true;
}

void expr_queue::push_front(const Expr& expr) {
	CH::str expr_str = expr_to_str(expr);
	if(*expr_str >= QUEUE_WIDTH)
		expr_str = expr_str(A | A + (QUEUE_WIDTH - 2));
	mvsaddstr(queue_display, --m_y_begin, 0, expr_str);
	queue_display->prepare_refresh();
}

bool expr_queue::pop_front() {
	if(m_y_begin == m_y_end)
		return false;
	queue_display->del_line(0, m_y_begin++);
	queue_display->prepare_refresh();
	return true;
}

CH::str expr_queue::expr_to_str(const Expr& expr) {
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
}