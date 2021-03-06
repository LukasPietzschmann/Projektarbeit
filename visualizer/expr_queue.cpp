#include "expr_queue.hpp"

expr_queue& expr_queue::the() {
	static expr_queue instance;
	return instance;
}

void expr_queue::push_back(const Expr& expr, bool is_comp) {
	auto er = expr_repr(expr, is_comp ? expr_repr::f_is_comp : 0);
	CH::str expr_str = truncate_string_to_length(er.as_string(), QUEUE_WIDTH - 2);
	mvsaddstr(queue_display, m_y_end, 0, expr_str(CH::A | CH::Z  - er.currpart_pos));
	wattron(**queue_display, A_MUTED);
	mvsaddstr(queue_display, m_y_end, er.currpart_pos, expr_str(CH::A + er.currpart_pos | CH::Z ));
	wattroff(**queue_display, A_MUTED);
	++m_y_end;
	queue_display->prepare_refresh();
}

bool expr_queue::pop_back() {
	if(m_y_begin == m_y_end)
		return false;
	queue_display->del_line(0, --m_y_end);
	queue_display->prepare_refresh();
	return true;
}

void expr_queue::push_front(const Expr& expr, bool is_comp) {
	auto er = expr_repr(expr, is_comp ? expr_repr::f_is_comp : 0);
	CH::str expr_str = truncate_string_to_length(er.as_string(), QUEUE_WIDTH - 2);
	--m_y_begin;
	mvsaddstr(queue_display, m_y_begin, 0, expr_str(CH::A | CH::Z  - er.currpart_pos));
	wattron(**queue_display, A_MUTED);
	mvsaddstr(queue_display, m_y_begin, er.currpart_pos, expr_str(CH::A + er.currpart_pos | CH::Z ));
	wattroff(**queue_display, A_MUTED);
	queue_display->prepare_refresh();
}

bool expr_queue::pop_front() {
	if(m_y_begin == m_y_end)
		return false;
	queue_display->del_line(0, m_y_begin++);
	queue_display->prepare_refresh();
	return true;
}