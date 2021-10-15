#include <iostream>
#include <regex>
#include <string>
using namespace std;

#include "data.h"
#include "scanner.h"
#include "parser.h"
#include "predef.h"
#include "eval.h"

// Ist val ein natürlicher Wert?
bool nat (Value val) {
    return val[intval_];
}

// Aktueller Kontext; zu Beginn der globale Kontext.
Context curr;

// Referenz auf das Value-Objekt im passenden Kontext liefern,
// im dem die Konstante oder der Operator oper gespeichert ist
// bzw. gespeichert werden soll.
Value& lookup (Oper oper) {
    // Den ersten Kontext c vom aktuellen Kontext nach außen suchen,
    // der einen Eintrag für den Operator oper enthält.
    // Wenn es keinen derartigen Kontext gibt,
    // wird der globale Kontext verwendet.
    Context c;
    for (c = curr; c(encl_); c = c(encl_)) {
	if (c(tab_).count(oper)) break;
    }

    // Auf den passenden Eintrag in der Variablentabelle dieses
    // Kontexts zugreifen.
    return c(tab_)[oper];
}

// Ausdruck expr auswerten.
Value eval (Expr expr) {
    // Auswertungsfunktion des Hauptoperators von expr aufrufen.
    return expr(oper_)(eval_)(expr);
}

// Hauptausdruck expr auswerten.
Value exec (Expr expr) {
    // Notwendige Vorbereitungen.
    curr = Context(tabptr_, make_shared<Tab>());
    curr(tab_)[type_oper] = uniq;
    curr(tab_)[bool_oper] = uniq;
    curr(tab_)[int_oper] = uniq;

    // Ausdruck auswerten.
    return eval(expr);
}

// Nacheinanderausführung expr auswerten.
Value sequ_eval (Expr expr) {
    eval(expr(row_)[A](opnd_));
    return eval(expr(row_)[Z](opnd_));
}

// Ausgabe expr auswerten.
Value print_eval (Expr expr) {
    // Breite w ermitteln.
    // Wenn der optionale Teil width w fehlt oder w ein unnatürlicher
    // Wert ist, bleibt w = 0.
    int w = 0;
    if (Expr width = expr(row_)[Z](passes_)[A](branch_)[Z](opnd_)) {
	w = eval(width)(intval_);
    }

    // Dezimaldarstellung des Werts x (bzw. "nichts") mit Breite w
    // erstellen und ausgeben.
    char buf [20];
    Value body = eval(expr(row_)[2*A](opnd_));
    if (nat(body)) sprintf(buf, "%*d", w, body(intval_));
    else sprintf(buf, "%*s", w, "");
    cout << buf << endl;

    return body;
}

// Konstantendeklaration expr auswerten.
Value cdecl_eval (Expr expr) {
    // Die von expr deklarierte Konstante
    // ist das einzige Element des Exportkatalogs von expr.
    Oper oper = expr(expt_)(opers_)[A];

    // Initialisierung der Konstanten auswerten
    // und im aktuellen Kontext speichern.
    return curr(tab_)[oper] = eval(oper(init_));
}

// Eindeutiger synthetischer Wert zur Repräsentation des Booleschen
// Werts true.
Value trueval = Value(synth_, true);

// Disjunktion oder Konjunktion expr auswerten.
Value logic_eval (Expr expr) {
    Value left = eval(expr(row_)[A](opnd_));
    char op = expr(row_)[2*A](word_)[A];
    if ((op == '|') == bool(left)) return left ? trueval : nil;
    return eval(expr(row_)[Z](opnd_)) ? trueval : nil;
}

// Negation expr auswerten.
Value neg_eval (Expr expr) {
    Value body = eval(expr(row_)[Z](opnd_));
    if (body) return nil;
    else return trueval;
}

// Fallunterscheidung expr auswerten.
Value branch_eval (Expr expr) {
    Value cond = eval(expr(row_)[2*A](opnd_));
    if (cond) return eval(expr(row_)[4*A](opnd_));

    for (Pass pass : expr(row_)[5*A](passes_)) {
	cond = eval(pass(branch_)[2*A](opnd_));
	if (cond) return eval(pass(branch_)[4*A](opnd_));
    }

    if (expr = expr(row_)[6*A](passes_)[A](branch_)[2*A](opnd_)) {
	return eval(expr);
    }

    return nil;
}

// Schleife expr auswerten.
Value loop_eval (Expr expr) {
    seq<Row> rows = expr(row_) + expr(row_)[3*A](passes_)(branch_());
    for (int n = 0; true; n++) {
	for (Row row : rows) {
	    char c = row[A](passes_)[A](branch_)[A](word_)[A];
	    Value val = eval(row[2*A](opnd_));
	    if (c == 'w' && !val || c == 'u' && val) {
		return Value(intval_, n);
	    }
	}
    }
}

// Eingabe expr auswerten.
Value read_eval (Expr expr) {
    // Eine Zeile s von der Standardeingabe lesen.
    // Resultat nil, falls dies fehlschlägt.
    string s;
    if (!getline(cin, s)) return nil;

    // Überprüfen, ob die Zeile eine syntaktisch korrekte ganze Zahl
    // enthält: ein optionales Minuszeichen, dann eine oder mehrere
    // Dezimalziffern.
    // Resultat nil, falls dies nicht der Fall ist.
    static regex re("[ \t]*-?[0-9]+[ \t]*");
    if (!regex_match(s, re)) return nil;

    // Die Zeile als Dezimalzahl interpretieren.
    return Value(intval_, stoi(s));
}

// (Ggf. verketteten) Vergleich expr auswerten.
Value cmp_eval (Expr expr) {
    // Den ersten Operanden rekursiv auswerten.
    Value left = eval(expr(row_)[A](opnd_));

    for (Row row :
	    expr(row_)(A+1|Z-1) + expr(row_)[4*A](passes_)(branch_())) {
	// Den nächsten Operator ermitteln
	// und den nächsten Operanden rekursiv auswerten.
	str oper = row[A](passes_)[A](branch_)[A](word_);
	Value right = eval(row[2*A](opnd_));

	// left und right gemäß oper vergleichen.
	int diff = left(intval_) - right(intval_);
	bool bothnat = nat(left) && nat(right);
	bool equal = left == right || bothnat && diff == 0;
	bool result = equal;

	switch (oper[A]) {
	case '<':
	    result = bothnat && diff < 0;
	    break;
	case '>':
	    result = bothnat && diff > 0;
	    break;
	}

	switch (oper[2*A]) {
	case '=':
	    result |= equal;
	    break;
	case '/':
	    result = !result;
	    break;
	}

	// Ggf. abbrechen.
	if (!result) return nil;

	left = right;
    }

    return trueval;
}

// Addition, Subtraktion, Multiplikation, Division oder Potenz expr
// auswerten.
Value bin_eval (Expr expr) {
    // Linken und rechten Operanden rekursiv auswerten.
    Value left = eval(expr(row_)[A](opnd_));
    Value right = eval(expr(row_)[Z](opnd_));

    // Ergebnis nil, wenn mindestens ein Operand ein unnatürlicher
    // Wert ist.
    if (!nat(left) || !nat(right)) return nil;

    // Andernfalls sind beide Operanden natürliche Werte.
    int x = left(intval_), y = right(intval_), z;

    // Fallunterscheidung anhand des Operatorsymbols.
    Item item = expr(row_)[2*A];
    char op = item(word_)[A];
    if (!op) op = item(passes_)[A](branch_)[A](word_)[A];
    switch (op) {
    case '+':
	// Addition.
	z = x + y;
	break;
    case '-':
	// Subtraktion.
	z = x - y;
	break;
    case '*':
	// Multiplikation.
	z = x * y;
	break;
    case '/':
	// Division mit Sonderfall y = 0.
	if (y == 0) return nil;
	z = x / y;
	break;
    case '^':
	// Potenz mit Fallunterscheidung anhand von x.
	switch (x) {
	case 1:
	    // 1 ^ y = 1 für alle y
	    z = 1;
	    break;
	case -1:
	    // (-1) ^ y = 1 für gerade y, sonst -1
	    z = y % 2 == 0 ? 1 : -1;
	    break;
	case 0:
	    // 0 ^ y = 1 / 0 ^ -y = 1 / 0 = nil für y < 0
	    //       = 1 für y = 0
	    //       = 0 für y > 0
	    if (y < 0) return nil;
	    z = y == 0 ? 1 : 0;
	    break;
	default:
	    // Der Betrag von x ist größer als 1.
	    // x ^ y = 1 / x ^ -y = 0 für y < 0
	    //       = 1 für y = 0
	    //       = x * ... * x für y > 0
	    if (y < 0) {
		z = 0;
		break;
	    }
	    z = 1;
	    while (y-- > 0) z *= x;
	    break;
	}
    }

    // Berechneten Wert z zurückgeben.
    return Value(intval_, z);
}

// Vorzeichenwechsel expr auswerten.
Value chs_eval (Expr expr) {
    Value body = eval(expr(row_)[Z](opnd_));
    if (!nat(body)) return nil;
    return Value(intval_, -body(intval_));
}

// Fakultät expr auswerten.
Value fac_eval (Expr expr) {
    Value body = eval(expr(row_)[A](opnd_));
    if (!nat(body)) return nil;

    int x = body(intval_);
    if (x < 0) return nil;

    int z = 1;
    while (x > 1) z *= x--;
    return Value(intval_, z);
}

// Klammerausdruck expr auswerten.
Value paren_eval (Expr expr) {
    return eval(expr(row_)[2*A](opnd_));
}

// Ganzzahliges Literal expr auswerten.
Value intlit_eval (Expr expr) {
    int val = 0;
    for (char c : expr(row_)[A](word_)) val = val * 10 + c - '0';
    return Value(intval_, val);
}

// Konstante expr auswerten.
Value const_eval (Expr expr) {
    return lookup(expr(oper_));
}
