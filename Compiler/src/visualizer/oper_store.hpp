#pragma once

#include <map>
#include <optional>

#include "../flxc/data.h"
#include "../libCH/seq.ch"

#include "constants.hpp"
#include "popup.hpp"
#include "utils.hpp"
#include "windows.hpp"

class oper_store {
public:
	static oper_store& the() {
		static oper_store instance;
		return instance;
	}

	oper_store(const oper_store&) = delete;
	oper_store(oper_store&&) noexcept = default;
	oper_store& operator=(const oper_store&) = delete;
	oper_store& operator=(oper_store&&) noexcept = default;

	bool insert_if_prototyp(const Expr& expr);
	std::optional<int> get_id_from_oper(const Oper& oper);

private:
	oper_store() = default;

	std::map<Oper, int> m_opers;
};