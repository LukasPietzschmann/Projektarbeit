#include "predef.h"
#include "eval.h"
#include "parser.h"

// Hilfsfunktionen zur Erzeugung von Signaturteilen aus irgendwelchen
// Dingen (Namen, Parameter oder Signaturteile).
Part part (str name) { return Part(name_, name); }
Part part (Par par) { return Part(par_, par); }
Part part (Part part) { return part; }

// Hilfsfunktionen zur Erzeugung von Signaturen aus irgendwelchen
// Dingen.
template <typename ... TT>
Sig sig (TT ... xx) { return Sig(part(xx) ...); }
Sig sig (Sig sig) { return sig; }

// Optionales oder wiederholbares Signaturteil mit einer Alternative
// liefern, die aus irgendwelchen Dingen besteht.
template <typename ... TT>
Part opt (TT ... xx) { return Part(opt_, true)(alts_, sig(xx ...)); }
template <typename ... TT>
Part rep (TT ... xx) { return opt(xx ...)(rep_, true); }

// Signaturteil liefern, das aus den Alternativen xx ... besteht.
template <typename ... TT>
Part alts (TT ... xx) { return Part(alts_, sig(xx) ...); }

// Operator mit Auswertungsfunktion eval erzeugen,
// dessen Signatur aus irgendwelchen Dingen besteht.
template <typename ... TT>
Oper oper (Eval eval, TT ... xx) {
    return Oper(eval_, eval)(sig_, sig(xx ...));
}

// Parameter mit dem willkürlichen Namen x liefern.
Par par () { return oper(nullptr, "x"); }

// Ausschlussangabe mit den Operatoren opers zum Parameter par
// hinzufügen. Die Attribute front, middle, back, left, top, right
// ergeben sich aus den entsprechenden Buchstaben von flags.
void excl (Par par, seq<Oper> opers, str flags) {
    Excl excl = Excl(expr_, Expr(expt_, cat(opers)));
    for (char c : flags) switch (c) {
    case 'F': excl(front_, true); break;
    case 'M': excl(middle_, true); break;
    case 'B': excl(back_, true); break;
    case 'L': excl(left_, true); break;
    case 'T': excl(top_, true); break;
    case 'R': excl(right_, true); break;
    }
    par(excls_, Z, excl);
}

// Importangabe mit den Operatoren opers zum Parameter par hinzufügen.
void impt (Par par, seq<Oper> opers) {
    par(impt_, Expr(expt_, cat(opers)));
}

// Exportangabe mit den Operatoren opers zum Operator oper hinzufügen.
void expt (Oper oper, seq<Oper> opers) {
    oper(expt_, Expr(expt_, cat(opers)));
}

// Wenn s mit einem Anführungszeichen beginnt (und dann auch endet),
// werden diese entfernt sowie zwei aufeinanderfolgende
// Anführungszeichen jeweils durch eines ersetzt.
str trim (str s) {
    if (s[A] != '"') return s;
    bool flag = false;
    return s(A+1|Z+1)
	([&] (char c) { return c == '"' ? flag = !flag : true; });
}

// Vom Ausdruck expr deklarierte Konstante erzeugen.
void create_const (Expr expr) {
    // Konstante als statischen Operator erzeugen.
    // Vor dem Doppelpunkt stehen die Namen der Konstanten,
    // nach dem Gleichheitszeichen ihre Initialisierung.
    Row row = expr(row_);
    Sig sig = seq(Part(name_, trim(row[A](word_))));
    for (Pass pass : row[2*A](passes_)) {
	sig += Part(name_, trim(pass(branch_)[A](word_)));
    }
    Oper oper = Oper(sig_, sig)(stat_, true)
		    (init_, expr(row_)[Z](opnd_))(eval_, const_eval);

    // Diesen Operator in den Exportkatalog der Deklaration schreiben.
    expr(expt_, cat(seq(oper)));
}

// Vordefinierte Typkonstanten.
// (Globale Variablen, damit sie in predef verfügbar sind.)
Oper type_oper, bool_oper, int_oper;

// Menge der vordefinierten Operatoren liefern.
seq<Oper> predef () {
    // Parameter.
    Par x, y, z;

    // Symbolliteral für Konstantendeklaration.
    Part sym = Part(name_, "\"([^\"\\s]|\"\")+\"|[A-Za-z][A-Za-z0-9]*")
							(reg_, true);

    // Operatoren unterschiedlicher Art definieren.
    // Wenn der Vorrang sukzessive zunimmt, kann man für
    // Ausschlussangaben immer die passende Kombination der Mengen
    // prefix, infix und postfix verwenden (prefix + infix für vordere
    // Operanden, infix + postfix für hintere).
    seq<Oper> prefix, infix, postfix, nullfix;

    // Metatyp type mit zugehörigem Operator.
    Type type_type = uniq;
    type_oper = Oper(res_, type_type)(sig_, Part(name_, "type"))
						(eval_, const_eval);
    type_type(type_, type_type)(oper_, type_oper);
    nullfix += type_oper;

    // Typ bool mit zugehörigem Operator.
    bool_oper = Oper(res_, type_type)(sig_, Part(name_, "bool"))
						(eval_, const_eval);
    Type bool_type = Type(type_, type_type)(oper_, bool_oper);
    nullfix += bool_oper;

    // Typ int mit zugehörigem Operator.
    int_oper = Oper(res_, type_type)(sig_, Part(name_, "int"))
						(eval_, const_eval);
    Type int_type = Type(type_, type_type)(oper_, int_oper);
    nullfix += int_oper;

    // Nacheinanderausführung.
    Oper sequ = oper(sequ_eval, x = par(), ";", y = par());
    infix += sequ;
    excl(y, infix + postfix, "BL");
    impt(y, seq(All, x));
    expt(sequ, seq(x, y));

    // Ausgabe.
    Oper pr = oper(print_eval,
			"print", x = par(), opt("width", y = par()));
    excl(x, infix + postfix, "BL");
    excl(x, infix, "MT"); // TODO: Ist T hier prinzipiell richtig?
    excl(y, infix + postfix, "BL");
    prefix += pr;

    // Konstantendeklaration.
    Oper cdecl = oper(cdecl_eval, sym, rep(sym), ":", "=", x = par());
    cdecl(sig_)[Z](after_, create_const);
    excl(x, infix + postfix, "BL");
    prefix += cdecl;

    // Disjunktion.
    Oper disj = oper(logic_eval, x = par(), "|", y = par());
    excl(x, prefix + infix, "FR");
    infix += disj;
    excl(y, infix + postfix, "BL");

    // Konjunktion.
    Oper conj = oper(logic_eval, x = par(), "&", y = par());
    excl(x, prefix + infix, "FR");
    infix += conj;
    excl(y, infix + postfix, "BL");

    // Negation.
    Oper neg = oper(neg_eval, "~", x = par());
    excl(x, infix + postfix, "BL");
    prefix += neg;

    // Vergleiche.
    Oper cmp = oper(cmp_eval,
	x = par(), alts("<", "<=", "=", "=/", ">=", ">"), y = par(),
	rep(alts("<", "<=", "=", "=/", ">=", ">"), z = par()));
    infix += cmp;
    excl(x, prefix + infix, "FR");
    excl(y, prefix + infix + postfix, "MLR");
    excl(z, prefix + infix + postfix, "MLR");
    excl(y, infix + postfix, "BL");
    excl(z, infix + postfix, "BL");

    // Addition und Subtraktion (linksassoziativ).
    Oper addsub = oper(bin_eval, x = par(), alts("+", "-"), y = par());
    excl(x, prefix + infix, "FR");
    infix += addsub;
    excl(y, infix + postfix, "BL");

    // Multiplikation und Division (linksassoziativ).
    Oper muldiv = oper(bin_eval, x = par(), alts("*", "/"), y = par());
    excl(x, prefix + infix, "FR");
    infix += muldiv;
    excl(y, infix + postfix, "BL");

    // Vorzeichenwechsel.
    Oper chs = oper(chs_eval, "-", x = par());
    excl(x, infix + postfix, "BL");
    prefix += chs;

    // Potenz (rechtsassoziativ).
    Oper pow = oper(bin_eval, x = par(), "^", y = par());
    excl(y, infix + postfix, "BL");
    infix += pow;
    excl(x, prefix + infix, "FR");

    // Fakultät.
    Oper fac = oper(fac_eval, x = par(), "!");
    excl(x, prefix + infix, "FR");
    postfix += fac;

    // Klammern.
    Oper paren = oper(paren_eval, "(", par(), ")");
    nullfix += paren;
    paren = oper(paren_eval, "{", par(), "}");
    nullfix += paren;

    // Ganzzahlige Literale.
    Oper intlit = Oper(eval_, intlit_eval)
		    (sig_, Part(name_, "[0-9][0-9]*")(reg_, true));
    nullfix += intlit;

    // Fallunterscheidung.
    Oper branch = oper(branch_eval, "if", par(), "then", par(),
	rep("elseif", par(), "then", par()), opt("else", par()), "end");
    nullfix += branch;

    // Schleife.
    Oper loop = oper(loop_eval, alts("while", "until", "do"), par(),
	rep(alts("while", "until", "do"), par()), "end");
    nullfix += loop;

    // Eingabe.
    Oper read = oper(read_eval, "read");
    nullfix += read;

    return prefix + infix + postfix + nullfix;
}
