// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef FLXC_SCANNER
#define FLXC_SCANNER 2021'04'27

#include "seq.ch"
using namespace CH;

extern str scan_str;
void scan_open (str filename);
void scan_white (posA& pos);
bool scan_exact (posA& pos, str name);
bool scan_match (posA& pos, str name, str& word);
bool scan_eof (posA pos);

#endif
