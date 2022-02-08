// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef FLXC_DATA
#define FLXC_DATA 2021'10'13

#include <functional>
using namespace std;

#define USING_CH
#include "seq.ch"
#include "open.ch"
using namespace CH;

// Typen.

// Ausdruck.
TYPE(Expr)

// Typ.
using Type = Expr;

// Operator.
TYPE(Oper)

// Parameter.
using Par = Oper;

// Teil einer Operatorsignatur.
TYPE(Part)

ATTR1 (currpart_, Expr, Part)

// Operatorsignatur.
using Sig = seq<Part>;

// Ausschlussangabe.
TYPE(Excl)

// Eintrag in einer Reihe.
TYPE(Item)

// Reihe von Einträgen.
using Row = seq<Item>;

// Ein Durchlauf durch eine Klammer einer Operatorsignatur.
TYPE(Pass)

// Operatorkatalog.
TYPE(Cat)

// Attribute von Ausdrücken.

// Operator des Ausdrucks.
ATTR1(oper_, Expr, Oper)

// Typ des Ausdrucks.
ATTR1(type_, Expr, Type)

// Reihe von Einträgen.
ATTRN(row_, Expr, Item)

// Import- und Exportkatalog.
ATTR1(impt_, Expr, Cat)
ATTR1(expt_, Expr, Cat)

// Anfangs- und Endposition im Quelltext.
ATTR1(beg_, Expr, posA)
ATTR1(end_, Expr, posA)

// Attribute von Operatoren.

// Signatur eines Operators.
ATTRN(sig_, Oper, Part)

// Resultattyp eines Operators.
ATTR1(res_, Oper, Type)

// Initialisierung oder Implementierung eines Operators.
ATTR1(init_, Oper, Expr)
ATTR1(impl_, Oper, Expr)

// Kennzeichnung eines Operators als statisch und/oder virtuell.
ATTR1(stat_, Oper, bool)
ATTR1(virt_, Oper, bool)

// Import-, Export- und Ausschlussangaben.
ATTR1(impt_, Oper, Expr)
ATTR1(expt_, Oper, Expr)
ATTRN(excls_, Oper, Excl)

// Attribute von Signaturteilen.

// Name.
ATTR1(name_, Part, str)

// Kennzeichnung eines Namens als regulären Ausdruck.
ATTR1(reg_, Part, bool)

// Parameter.
ATTR1(par_, Part, Par)

// Alternativen in einer Klammer.
ATTRN(alts_, Part, Sig)

// Kennzeichnung einer Klammer als optional und/oder wiederholbar.
ATTR1(opt_, Part, bool)
ATTR1(rep_, Part, bool)

// Prolog- und Epilogfunktion, die vor bzw. nach der Verarbeitung des
// Signaturteils durch den Parser ausgeführt wird.
ATTR1(before_, Part, function<void (Expr)>)
ATTR1(after_, Part, function<void (Expr)>)

// Attribute von Einträgen in einer Reihe.

// Die zu einem Namen gehörende Zeichenfolge.
ATTR1(word_, Item, str)

// Der zu einem Parameter gehörende Operand.
ATTR1(opnd_, Item, Expr)

// Die zu einer Klammer gehörenden Durchläufe.
ATTRN(passes_, Item, Pass)

// Attribute von Durchläufen.

// Position der durchlaufenen Alternative der zugehörigen Klammer
// in der Folge aller Alternativen.
// 0*A, wenn eine eckige Klammer gar nicht durchlaufen wurde.
ATTR1(choice_, Pass, posA)

// Reihe von Einträgen für die durchlaufene Alternative der Klammer.
ATTRN(branch_, Pass, Item)

// Attribute von Ausschlussangaben.

// Zugehöriger Ausdruck.
ATTR1(expr_, Excl, Expr)

// Gilt die Ausschlussangabe für den vorderen und/oder einen mittleren
// und/oder den hinteren Operanden eines Ausdrucks?
ATTR1(front_, Excl, bool)
ATTR1(middle_, Excl, bool)
ATTR1(back_, Excl, bool)

// Bezieht sich die Ausschlussangabe auf den linken Rand und/oder die
// Spitze und/oder den rechten Rand eines Operanden?
ATTR1(left_, Excl, bool)
ATTR1(top_, Excl, bool)
ATTR1(right_, Excl, bool)

ATTR (to_str_from_currpart_)
inline str FUNC (to_str_from_currpart_, Expr expr) {
	std::function<str(const Sig&, bool)> get_ident = [&get_ident, &expr](const Sig& sig, bool skip)->str {
		str res;
		bool ab_jetzt = !skip;
		for(const Part& part: sig) {
			if(expr(currpart_) == part) {
				ab_jetzt = true;
			}
			if(part(opt_) && ab_jetzt)
				res += "[";
			if(const str& name = part(name_)) {
				if(ab_jetzt)
					res += name + " ";
			}
			else if(const Oper& param = part(par_)){ //res += get_ident(param(sig_));
				if(ab_jetzt)
					res += "_ ";
			}
			else if(const seq<Sig>& alts = part(alts_)) {
					for(const Sig& s: alts)
						res += get_ident(s, !ab_jetzt);
			}
			if(part(opt_) && ab_jetzt)
				res += "]";
		}
		return res;
	};

	return get_ident(expr(oper_)(sig_), true);
}

ATTR (to_str_)
inline str FUNC (to_str_, Expr expr) {
	std::function<str(const Sig&)> get_ident = [&get_ident, &expr](const Sig& sig)->str {
		str res;
		for(const Part& part: sig) {
			if(part(opt_))
				res += "[";
			if(const str& name = part(name_))
					res += name + " ";
			else if(const Oper& param = part(par_)) //res += get_ident(param(sig_));
					res += "_ ";
			else if(const seq<Sig>& alts = part(alts_)) {
					for(const Sig& s: alts)
						res += get_ident(s);
			}
			if(part(opt_))
				res += "]";
		}
		return res;
	};

	return get_ident(expr(oper_)(sig_));
}

// Operatorkataloge.

// Die in einem Katalog enthaltenen Operatoren als virtuelles Attribut,
// das nur abgefragt, aber nicht verändert werden kann.
ATTR(opers_)
seq<Oper> FUNC (opers_, Cat c);

Cat cat (const seq<Oper>& os = seq<Oper>());
bool operator<< (Oper oper, Cat cat);
Cat& operator+= (Cat& cat1, Cat cat2);
Cat intersect (Cat cat1, Cat cat2, seq<Oper>& diff);

#endif
