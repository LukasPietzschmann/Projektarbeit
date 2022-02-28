#include "oper_store.hpp"

oper_store& oper_store::the() {
	static oper_store instance;
	return instance;
}

bool oper_store::insert_if_prototyp(const Expr& expr) {
	static int next_id_and_line = 0;

	if(expr(beg_) != expr(end_))
		return false;

	if(m_opers.find(expr(oper_)) != m_opers.end())
		return false;

	m_opers.try_emplace(expr(oper_), next_id_and_line);

	expr_repr er = expr_repr(expr, expr_repr::f_is_prototype);

	mvpaddstr(opers_popup, next_id_and_line, 0, er.as_string(POPUP_WIDTH - 1));
	++next_id_and_line;

	return true;
}

std::optional<int> oper_store::get_id_from_oper(const Oper& oper) {
	const auto& it = m_opers.find(oper);
	if(it == m_opers.end())
		return {};

	return it->second;
}