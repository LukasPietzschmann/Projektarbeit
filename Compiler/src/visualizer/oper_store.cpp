#include "oper_store.hpp"

bool oper_store::insert_if_prototyp(const Expr& expr) {
	static int next_id_and_line = 0;

	if(expr(beg_) != expr(end_))
		return false;

	if(m_opers.find(expr(oper_)) != m_opers.end())
		return false;

	m_opers.try_emplace(expr(oper_), next_id_and_line);

	std::stringstream ss;
	ss << std::to_string(next_id_and_line) << ": " << expr(to_str_);
	CH::str entry(ss.str());

	if(*entry >= POPUP_WIDTH)
		entry = entry(A | A + (POPUP_WIDTH - 5)) + "...";

	mvpaddstr(opers_popup, next_id_and_line, 0, entry);
	++next_id_and_line;

	return true;
}

std::optional<int> oper_store::get_id_from_oper(const Oper& oper) {
	const auto& it = m_opers.find(oper);
	if(it == m_opers.end())
		return {};

	return it->second;
}