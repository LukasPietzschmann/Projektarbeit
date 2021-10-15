// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef CH_FUNC_CH
#define CH_FUNC_CH 2021'03'22

#include <utility>

namespace CH {

// Basistyp aller im folgenden definierten Typen F, der die Operatoren
// Negation (!), Konjunktion (&&), Disjunktion (||) und Komposition (,)
// für alle Funktionsobjekte definiert, sodass ein Ausdruck wie z. B.
// (rem(2), eq(0)) && (rem(4), !eq(0)) ein Funktionsobjekt konstruiert,
// das die Funktion f(x) = x%2 == 0 && !(x%4 == 0) implementiert.
// Die Funktionsschablonen neg, conj, disj und comp werden später
// definiert.
template <typename F>
struct CH_common_t {
    // Das aktuelle Objekt mit seinem eigentlichen Typ F.
    F& self () const {
	return *(F*)this;
    }

    // Negation.
    auto operator! () const {
	return neg(self());
    }

    // Konjunktion.
    template <typename F2>
    auto operator&& (const F2& f2) const {
	return conj(self(), f2);
    }

    // Disjunktion.
    template <typename F2>
    auto operator|| (const F2& f2) const {
	return disj(self(), f2);
    }

    // Komposition.
    template <typename F2>
    auto operator, (const F2& f2) const {
	return comp(f2, self());
    }
};

// Die Funktionsschablone neg kann mit einem Funktionsobjekt f eines
// beliebigen Typs F aufgerufen werden und liefert als Resultat ein
// Objekt des Hilfstyps neg_t<F>, das das Objekt f enthält.
// Die Elementfunktion operator() dieses Typs kann mit beliebigen
// Werten xx ... aufgerufen werden und liefert als Resultat den Wert
// !f(std::forward<XX>(xx) ...).

template <typename F>
struct neg_t : CH_common_t<neg_t<F>> {
    F f;
    neg_t (const F& f) : f(f) {}

    template <typename ... XX>
    auto operator() (XX&& ... xx) const {
	return !f(std::forward<XX>(xx) ...);
    }
};

template <typename F>
auto neg (const F& f) {
    return neg_t<F>(f);
}

// Für einen Namen name (z. B. conj) und einen zugehörigen Ausdruck expr
// (z. B. f1(std::forward<XX>(xx) ...) && f2(std::forward<XX>(xx) ...))
// definiert AUX(name, expr) eine Funktionsschablone name (z. B. conj),
// die mit zwei Funktionsobjekten f1 und f2 mit beliebigen Typen F1 und
// F2 aufgerufen werden kann und als Resultat ein Objekt des Hilfstyps
// name_t (z. B. conj_t) liefert, das die Objekte f1 und f2
// enthält.
// Die Elementfunktion operator() dieses Typs kann mit beliebigen Werten
// xx ... aufgerufen werden und liefert als Resultat den Wert expr.
#define AUX(name, expr) \
    template <typename F1, typename F2> \
    struct name##_t : CH_common_t<name##_t<F1, F2>> { \
	F1 f1; \
	F2 f2; \
	name##_t (const F1& f1, const F2& f2) : f1(f1), f2(f2) {} \
	\
	template <typename ... XX> \
	auto operator() (XX&& ... xx) const { \
	    return expr; \
	} \
    }; \
    template <typename F1, typename F2> \
    auto name (const F1& f1, const F2& f2) { \
	return name##_t<F1, F2>(f1, f2); \
    }

// Verwendung von AUX für Konjunktion, Disjunktion und Komposition.
AUX(conj, f1(std::forward<XX>(xx) ...) && f2(std::forward<XX>(xx) ...))
AUX(disj, f1(std::forward<XX>(xx) ...) || f2(std::forward<XX>(xx) ...))
AUX(comp, f1(f2(std::forward<XX>(xx) ...)))

#undef AUX

// Für einen Namen name (z. B. conj) definiert AUX(name) eine
// variadische Funktion name (z. B. conj) als Verallgemeinerung der
// gleichnamigen zweistelligen Funktion, die mit zwei oder mehr
// Funktionsobjekten f1, f2, ff ... aufgerufen werden kann.
// Da bei einem Aufruf mit genau zwei Funktionsobjekten f1, f2 die
// nicht-variadische zweistellige Funktion bevorzugt wird, wird die
// hier definierte variadische Funktion tatsächlich immer mit mindestens
// drei Funktionsobjekten aufgerufen, d. h. das Bündel ff ist niemals
// leer.
#define AUX(name) \
    template <typename F1, typename F2, typename ... FF> \
    auto name (const F1& f1, const F2& f2, const FF& ... ff) { \
	return name(name(f1, f2), ff ...); \
    }

// Verwendung von AUX für Konjunktion, Disjunktion und Komposition.
AUX(conj)
AUX(disj)
AUX(comp)

#undef AUX

// Für einen Namen name (z. B. eq) und ein zugehöriges Operatorsymbol
// oper (z. B. ==) definiert AUX(name, oper) eine Funktionsschablone
// name (z. B. eq), die mit einem Wert y eines beliebigen Typs Y
// aufgerufen werden kann und als Resultat ein Objekt des Hilfstyps
// name_t<Y> (z. B. eq_t<Y>) liefert, das den Wert y enthält.
// Die Elementfunktion operator() dieses Typs kann mit einem Wert x
// eines beliebigen Typs X aufgerufen werden und liefert als Resultat
// den Wert x oper y (z. B. x == y).
#define AUX(name, oper) \
    template <typename Y> \
    struct name##_t : CH_common_t<name##_t<Y>> { \
	Y y; \
	name##_t (const Y& y) : y(y) {} \
	\
	template <typename X> \
	auto operator() (const X& x) const { \
	    return x oper y; \
	} \
    }; \
    \
    template <typename Y> \
    name##_t<Y> name (const Y& y) { \
	return name##_t<Y>(y); \
    }

// Verwendung von AUX für Vergleichsoperatoren.
AUX(eq, ==)
AUX(ne, !=)
AUX(gt, >)
AUX(ge, >=)
AUX(lt, <)
AUX(le, <=)

// Verwendung von AUX für arithmetische Operatoren.
AUX(add, +)
AUX(sub, -)
AUX(mul, *)
AUX(div, /)
AUX(rem, %)

#undef AUX

}

#endif
