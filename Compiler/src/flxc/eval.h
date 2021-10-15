#ifndef FLXC_EVAL
#define FLXC_EVAL 2021'10'15

#include <memory>
#include <unordered_map>

#define USING_CH
#include "open.ch"
using namespace CH;

#include "data.h"

// Laufzeitwert.
TYPE(Value)

// Ganzzahliger Wert.
ATTR1(intval_, Value, int)

// Kennzeichnung eines synthetischen Werts.
ATTR1(synth_, Value, bool)

// Typ von Auswertungsfunktionen.
using Eval = Value (*) (Expr);

// Auswertungsfunktion eines Operators.
ATTR1(eval_, Oper, Eval)

// Kontext.
TYPE(Context)

// Statisch umschließender Kontext.
ATTR1(encl_, Context, Context)

// Tabelle mit den Werten aller Konstanten und Operatoren
// eines Kontexts.
HASH(Oper)
using Tab = unordered_map<Oper, Value>;
ATTR1(tabptr_, Context, shared_ptr<Tab>)
ATTR(tab_)
inline Tab& FUNC (tab_, Context c) { return *c(tabptr_); }

// Wenn die Speicherbereinigung für offene Typen verwendet wird:
#if CH_GC

// Alle in tab enthaltenen Objekte offener Typen ermitteln
// und an die Funktion f übergeben.
template <typename F>
void CH::follow (const shared_ptr<Tab>& tab, const F& f) {
    for (auto& x : *tab) {
	CH::follow(x.first, f);
	CH::follow(x.second, f);
    }
}

#endif

bool nat (Value val);
Value exec (Expr expr);
Value eval (Expr expr);
Value sequ_eval (Expr expr);
Value print_eval (Expr expr);
Value cdecl_eval (Expr expr);
Value logic_eval (Expr expr);
Value neg_eval (Expr expr);
Value branch_eval (Expr expr);
Value loop_eval (Expr expr);
Value read_eval (Expr expr);
Value cmp_eval (Expr expr);
Value bin_eval (Expr expr);
Value chs_eval (Expr expr);
Value fac_eval (Expr expr);
Value paren_eval (Expr expr);
Value intlit_eval (Expr expr);
Value const_eval (Expr expr);

#endif
