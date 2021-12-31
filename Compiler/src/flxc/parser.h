// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef FLXC_PARSER
#define FLXC_PARSER 2021'06'19

#include <sstream>
#include "data.h"
#include "scanner.h"
#include "../visualizer/events.hpp"

// Archiv.
TYPE (Arch)

ATTRN (cons_, Arch, Expr)
ATTRN (comp_, Arch, Expr)
ATTR1 (excl_, Arch, Cat)
ATTR1 (pos_, Arch, posA)

template <>
struct std::hash<Cat> : CH::hash<Cat> {};

// Schlüssel zur Identifizierung von Archiven.
using Key = pair<posA, Cat>;

template <>
struct std::hash<Key> {
	size_t operator()(Key key) const {
		return key.first - A | std::hash<Cat>()(key.second) << 16;
	}
};

// inline ist wichtig, damit die Objekte All und Self in allen
// Übersetzungseinheiten gleich sind.
inline const Oper All = uniq;
inline const Oper Self = uniq;

void setback (const Sig& sig, bool3 back = true);

seq<Expr> parse (const seq<Oper>& opers);

void print (Expr expr, str ind = "");

// Löschen der gesammten Archiv-Tabelle.
void clear_tab();

#endif

