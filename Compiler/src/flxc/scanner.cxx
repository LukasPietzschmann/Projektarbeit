// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>
using namespace std;

#include "scanner.h"

// Inhalt der von scan_open gelesenen Quelldatei.
str scan_str;

// Datei filename öffnen und ihren gesamten Inhalt in scan_str speichern.
void scan_open (str filename) {
    ifstream f (string(filename(A), ~filename(Z)));
    if (!f) {
	std::cerr << "Cannot open input file: " << filename << endl;
	exit(1);
    }
    f.seekg(0);
    scan_str = "";
    char c;
    while (f.get(c)) scan_str += c;
}

// Gesamten Inhalt aus `string` in `scan_srt` einfügen.
// Bisher nur vom REPL verwendet.
void scan_string(str string){
    scan_str = string;
}

// Zwischenraum lesen.
void scan_white (posA& pos) {
    while (isspace(scan_str[pos])) pos++;
}

// Die Zeichenfolge name lesen.
bool scan_exact (posA& pos, str name) {
    int n = *name;
    if (scan_str(pos|n) != name) return false;
    pos += n;
    return true;
}

// Ein Wort word lesen, das auf den regulären Ausdruck name passt.
bool scan_match (posA& pos, str name, str& word) {
    try {
	// match_continuous bedeutet, dass die auf das Muster passende
	// Zeichenfolge nur an der Anfangsposition pos beginnen darf.
	regex re (name(A), ~name(Z));
	match_results<decltype(scan_str(A))> rs;
	if (!regex_search(scan_str(pos), ~scan_str(Z), rs, re,
				regex_constants::match_continuous)) {
	    return false;
	}
	
	// Die Hilfsvariable w ist notwendig, damit begin() und end()
	// auf dasselbe Objekt angewandt werden!
	std::string w = rs.str();
	word = str(w.begin(), w.end());
	pos += *word;
	return true;
    }
    catch (regex_error e) {
	std::cerr << "Regular expression error: " << e.code() << endl;
	exit(1);
    }
}

// Ende der Eingabe lesen.
bool scan_eof (posA pos) {
    return pos == A + *scan_str;
}

str get_scanned_str_for_pos(posA start, posA end) {
	return scan_str(start | end);
}

str get_scanned_str_for_expr(Expr expr) {
	if(expr(beg_) == expr(end_))
		return expr(to_str_) + " (Prototyp)";
	return get_scanned_str_for_pos(expr(beg_), expr(end_));
}