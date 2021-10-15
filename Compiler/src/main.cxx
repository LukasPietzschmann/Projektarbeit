#define FLXC_MAIN 2021'10'15

#include <iostream>
using namespace std;

#include "scanner.h"
#include "predef.h"
#include "parser.h"
#include "eval.h"

// Hauptprogramm.
int main (int argc, char* argv []) {
    // Quelldatei öffnen und einlesen.
    str filename = argv[1];
    scan_open(filename);

    // Vordefinierte Operatoren erzeugen.
    seq<Oper> opers = predef();

    // Parser ausführen.
    seq<Expr> exprs = parse(opers);

    // Erkannte Ausdrücke ausgeben und auswerten.
    int i = 0;
    for (Expr expr : exprs) {
	Value val = exec(expr); // exec statt eval!
	cout << "Expression " << ++i << " with value ";
	if (nat(val)) cout << val(intval_);
	else if (val) cout << "synth";
	else cout << "nil";
	cout << endl;
    }
}
