// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef FLXC_PARSER
#define FLXC_PARSER 2021'06'19

#include "data.h"

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

