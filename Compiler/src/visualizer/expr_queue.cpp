#include "expr_queue.hpp"

void expr_queue::push(const Expr& expr) {
	CH::str expr_str = expr(to_str_);
	if(*expr_str >= QUEUE_WIDTH)
		expr_str = expr_str(A | A + (QUEUE_WIDTH - 2));
	mvwaddnstr(queue_display, m_size++, 0, &expr_str.elems[0], *expr_str);
	wrefresh(queue_display);
}

bool expr_queue::pop() {
	if(m_size == 0)
		return false;
	wmove(queue_display, --m_size, 0);
	wclrtobot(queue_display);
	wrefresh(queue_display);
	return true;
}