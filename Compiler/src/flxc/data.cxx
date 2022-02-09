// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#include <map>
#include <set>
using namespace std;

#include "data.h"

// Die in einem Katalog enthaltenen Operatoren als privates Attribut.
ATTRN(priv_opers_, Cat, Oper)

// Lesefunktion des zugehörigen virtuellen Attributs opers.
seq<Oper> FUNC (opers_, Cat c) {
    return c(priv_opers_);
}

// "Künstlicher" Vergleichsoperator für Operatoren,
// damit set<Oper> korrekt funktioniert.
// (set und map haben gegenüber unordered_set und unordered_map den
// Vorteil, dass operator< für set bereits definiert ist, sodass
// set<Oper> direkt als Schlüssel für map verwendet werden kann.)
bool operator< (Oper oper1, Oper oper2) {
    return oper1.id < oper2.id;
}

// Verzeichnis aller bis jetzt erzeugten Kataloge.
// (Verpackung in eine Funktion mit einer statischen Variablen,
// damit der Konstruktor garantiert bei der ersten Verwendung
// ausgeführt wird.)
map<set<Oper>, Cat>& catdir () {
    static map<set<Oper>, Cat> catdir;
    return catdir;
}

// Eindeutigen Katalog mit den Operatoren os liefern.
Cat cat (const seq<Oper>& os) {
    // Der leere Katalog wird durch nil repräsentiert,
    // damit er mit einem nicht vorhandenen Katalog übereinstimmt.
    if (!os) return nil;

    // Folge os in eine Menge umwandeln und als Schlüssel verwenden.
    // Da äquivalente Operatoren momentan nicht beachtet werden,
    // spielt die Reihenfolge keine Rolle.
    // Nur wenn das Verzeichnis noch keinen Eintrag für diese Menge
    // enthält, wird ein neues Cat-Objekt erzeugt.
    Cat& c = catdir()[set<Oper>(os(A), ~os(Z))];
    if (!c) c = Cat(priv_opers_, os);
    return c;
}

// Ist der Operator oper im Katalog cat enthalten?
bool operator<< (Oper oper, Cat cat) {
    return cat(opers_)(searchA(eq(oper)));
}

// Operatoren des Katalogs cat2 zum Katalog cat1 hinzufügen.
Cat& operator+= (Cat& cat1, Cat cat2) {
    return cat1 = cat(cat1(opers_) + cat2(opers_));
}

// Schnittmenge (Resultat) und Differenz (Referenzparameter diff)
// der Kataloge cat1 und cat2 bilden.
Cat intersect (Cat cat1, Cat cat2, seq<Oper>& diff) {
    // Die Hilfsvariablen opers1 und opers2 werden gebraucht,
    // damit sich opers1(A) und opers1(Z) auf dieselbe Sequenz beziehen,
    // was bei cat1(opers_)(A) und cat1(opers_)(Z) nicht der Fall wäre!
    seq<Oper> opers1 = cat1(opers_);
    seq<Oper> opers2 = cat2(opers_);
    set<Oper> s1(opers1(A), ~opers1(Z));
    set<Oper> s2(opers2(A), ~opers2(Z));

    seq<Oper> inter;
    diff = seq();
    for (Oper oper : s1) {
	if (s2.count(oper)) inter += oper;
	else diff += oper;
    }
    return cat(inter);
}

str expr_to_str_int_(const Expr& expr, const Sig& sig, bool skip) {
	str res;
	for(const Part& part: sig) {
		if(expr(currpart_) == part)
			skip = false;
		if(part(opt_) && !skip)
			res += "[";
		if(const str& name = part(name_)) {
			if(!skip)
				res += name + " ";
		}else if(const Oper& param = part(par_)) { //res += get_ident(param(sig_));
			if(!skip)
				res += "_ ";
		}else if(const seq<Sig>& alts = part(alts_)) {
			for(const Sig& s: alts)
				res += expr_to_str_int_(expr, s, skip);
		}
		if(part(opt_) && !skip)
			res += "]";
	}
	return res;
}