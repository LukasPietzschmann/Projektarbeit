#include "expr_queue.hpp"

void expr_queue::push(const Expr& expr) {
	CH::str expr_str = expr(to_str_);
	if(*expr_str >= QUEUE_WIDTH)
		expr_str = expr_str(A | A + (QUEUE_WIDTH - 2));
	mvsaddstr(queue_display, m_y_end++, 0, expr_str);
	queue_display->prepare_refresh();
}

bool expr_queue::pop() {
	if(m_y_begin == m_y_end)
		return false;
	queue_display->del_line(0, m_y_begin++);
	queue_display->prepare_refresh();
	return true;
}