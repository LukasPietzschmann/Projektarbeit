#include "expr_queue.hpp"

void expr_queue::push(const Expr& expr) {
	CH::str expr_str = expr(to_str_);
	if(*expr_str >= QUEUE_WIDTH)
		expr_str = expr_str(A | A + (QUEUE_WIDTH - 2));
	mvwaddnstr(queue_display_pad, m_size++, 0, &expr_str.elems[0], *expr_str);
	prefresh(queue_display_pad, m_scroll_y, 0, 0, width - QUEUE_WIDTH, height - 1, width - 1);
}

bool expr_queue::pop() {
	if(m_size == 0)
		return false;
	wmove(queue_display_pad, --m_size, 0);
	wclrtobot(queue_display_pad);
	prefresh(queue_display_pad, m_scroll_y, 0, 0, width - QUEUE_WIDTH, height - 1, width - 1);
	return true;
}

//TODO return false if it could not be scrolled
void expr_queue::scroll_y(int delta) {
	if(m_scroll_y < 0)
		m_scroll_y = 0;
	else
		m_scroll_y += delta;
	prefresh(queue_display_pad, m_scroll_y, 0, 0, width - QUEUE_WIDTH, height - 1, width - 1);
}