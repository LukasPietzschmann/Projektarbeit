// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef FLXC_SCANNER
#define FLXC_SCANNER 2021'04'27

#include "seq.ch"
#include "data.h"
using namespace CH;

extern str scan_str;
void scan_open (str filename);
void scan_string(str string);
void scan_white (posA& pos);
bool scan_exact (posA& pos, str name);
bool scan_match (posA& pos, str name, str& word);
bool scan_eof (posA pos);
str get_scanned_str_for_pos(posA, posA);
str get_scanned_str_for_expr(Expr);

#endif
