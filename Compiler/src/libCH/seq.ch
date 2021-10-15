// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef CH_SEQ_CH
#define CH_SEQ_CH 2021'08'20

#include <algorithm>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "meta.ch"
#include "util.ch"
#include "func.ch"

namespace CH {

/*
 *  Hilfstypen und zugehörige Operationen
 */

// Hilfstyp, von dem Typen wie posAZ und iterAZ erben können,
// wenn sie sich selbst als Typparameter T angeben.
// Wenn es für einen solchen Typ dann T& operator+= (T, int) und
// int operator- (T, T) gibt, werden alle weiteren Operatoren, die sich
// damit implementieren lassen, von diesem Hilfstyp bereitgestellt.
// Aber Achtung: Wenn operator- als Elementfunktion in T definiert wird,
// muss dort using opers<T>::operator- angegeben werden, damit auch der
// hier definierte operator- gefunden wird.
template <typename T>
struct CH_opers_t {
    // Das aktuelle Objekt mit seinem eigentlichen Typ T.
    T& self () const {
	return *(T*)this;
    }

    // Additionsoperatoren.
    T& operator++ () {
	return self() += 1;
    }
    T operator++ (int) {
	T tmp = self();
	++self();
	return tmp;
    }
    T operator+ (int n) const {
	T tmp = self();
	return tmp += n;
    }
    friend T operator+ (int n, T self) {
	return self + n;
    }
    // Der vorige Operator wird durch "argument-dependent name lookup"
    // gefunden, weil CH_opers_t dann eine "associated class" von T ist.

    // Subtraktionsoperatoren.
    T& operator-= (int n) {
	return self() += -n;
    }
    T& operator-- () {
	return self() -= 1;
    }
    T operator-- (int) {
	T tmp = self();
	--self();
	return tmp;
    }
    T operator- (int n) const {
	T tmp = self();
	return tmp -= n;
    }

    // Vergleichsoperatoren.
    bool operator< (T other) const { return self() - other < 0; }
    bool operator> (T other) const { return self() - other > 0; }
    bool operator<= (T other) const { return self() - other <= 0; }
    bool operator>= (T other) const { return self() - other >= 0; }
    bool operator== (T other) const { return self() - other == 0; }
    bool operator!= (T other) const { return self() - other != 0; }
};

// Positionen posA = posAZ<true> und posZ = posAZ<false>
// relativ zum Anfang bzw. Ende einer beliebigen Sequenz.
template <bool AZ>
struct posAZ : CH_opers_t<posAZ<AZ>> {
    // Vielfaches der Referenzposition A bzw. Z.
    // A bzw. Z besitzen also Faktor 1.
    // Damit ist factor - 1 die Differenz zu A bzw. Z.
    int factor = 0;

    // Implizite Umwandlung nach bool(like) liefert genau dann false,
    // wenn sich die Position links von A bzw. rechts von Z befindet
    // und damit garantiert außerhalb jeder Sequenz liegt.
    operator boollike () const {
	return boollikeval(factor > 0);
    }

    // Summe der aktuellen Position und der ganzen Zahl n.
    posAZ<AZ>& operator+= (int n) {
	factor += n;
	return *this;
    }

    // Differenz der aktuellen Position und der Position other.
    int operator- (posAZ<AZ> other) const {
	return factor - other.factor;
    }

    // operator- des Hilfstyps CH_opers_t sichtbar machen.
    using CH_opers_t<posAZ<AZ>>::operator-;

    // Produkt der aktuellen Position und der ganzen Zahl n.
    // (Wird üblicherweise nur für die Randpositionen A und Z verwendet.)
    posAZ<AZ>& operator*= (int n) {
	factor *= n;
	return *this;
    }
    posAZ<AZ> operator* (int n) const {
	posAZ<AZ> tmp = *this;
	return tmp *= n;
    }
    friend posAZ<AZ> operator* (int n, posAZ<AZ> p) {
	return p * n;
    }

    // Quotient der aktuellen Position und der Position other.
    // (Üblicherweise werden nur die Randpositionen A und Z als
    // Divisor verwendet.)
    int operator/ (posAZ<AZ> other) {
	return factor / other.factor;
    }
};
using posA = posAZ<true>;
using posZ = posAZ<false>;

// Konstanten A und Z.
const posA A = posA() + 1;
const posZ Z = posZ() + 1;

// Bereich als Paar von Anfangs- und Endposition.
template <bool AZ1, bool AZ2>
using CH_range = std::pair<posAZ<AZ1>, posAZ<AZ2>>;

// Bereich mit Anfangsposition p und Endposition q.
template <bool AZ1, bool AZ2>
CH_range<AZ1, AZ2> operator| (posAZ<AZ1> p, posAZ<AZ2> q) {
    return std::make_pair(p, q);
}

// Bereich mit Anfangsposition p und Länge n.
inline CH_range<true, true> operator| (posAZ<true> p, int n) {
    return p | p + n;
}
inline CH_range<false, false> operator| (posAZ<false> p, int n) {
    return p | p - n;
}

// Bereich mit Länge n und Endposition q.
inline CH_range<true, true> operator| (int n, posAZ<true> q) {
    return q - n | q;
}
inline CH_range<false, false> operator| (int n, posAZ<false> q) {
    return q + n | q;
}

// Hilfstypen boolA = boolAZ<true> und boolZ = boolAZ<false>
// als "Repliken" von bool.
template <bool AZ>
struct boolAZ {
    bool value;
    boolAZ (bool x) : value(x) {}
    operator bool () const { return value; }
};
using boolA = boolAZ<true>;
using boolZ = boolAZ<false>;

// Hilfstypen bool12A = bool12<true> und bool12Z = bool12<false>
// als "Repliken" von bool mit einem zweiten optionalen bool-Wert.
template <bool AZ>
struct bool12AZ : std::pair<bool, std::optional<bool>> {
    bool12AZ (bool x) : pair(x, std::nullopt) {}
    bool12AZ (bool x, bool y) : pair(x, y) {}
    operator bool () const { return first; }
    std::optional<bool> operator* () const { return second; }
};
using bool12A = bool12AZ<true>;
using bool12Z = bool12AZ<false>;

/*
 *  Hilfsfunktionen
 */

// Position p relativ zum Anfang normalisieren, d. h. ihre Differenz
// zur Position A auf den Bereich von 0 bis N einschränken.
inline int CH_normalize (posAZ<true> p, int N) {
    int d = p.factor - 1;
    if (d < 0) return 0;
    else if (d > N) return N;
    else return d;
}

// Position p relativ zum Ende normalisieren
// und in eine Position relativ zum Anfang umrechnen.
inline int CH_normalize (posAZ<false> p, int N) {
    int d = p.factor - 1;
    if (d < 0) return N;
    else if (d > N) return 0;
    else return N - d;
}

/*
 *  Sequenztyp inklusive aller Operationen
 */

// Die Vorabdeklaration ist nötig, damit seq<void> schon vor seiner
// Verwendung in seq<T> spezialisiert werden kann.
template <typename T> struct seq;

// seq<void> repräsentiert eine leere Sequenz mit beliebigem Elementtyp.
template <>
struct seq <void> {};

// Sequenz mit Elementtyp T.
template <typename T>
struct seq {
    // Elementtyp.
    using elem_t = T;

    // Elemente.
    std::vector<T> elems;

    // Initialisierung als leere Sequenz.
    seq () : elems() {}
    seq (seq<void>) : elems() {}

    // Initialisierung mit einem oder mehreren Elementen xx ...,
    // deren Typen nach T umwandelbar ist.
    template <typename ... XX, CH_IF(CH_CONV(XX, T) ...)>
    explicit seq (XX&& ... xx) : elems() {
	(elems.push_back(std::forward<XX>(xx)), ...);
    }

    // Initialisierung mit den Elementen der Initialisiererliste xs.
    seq (std::initializer_list<T> xs) : elems(xs) {}

    // Initialisierung mit den Elementen von xs.
    // (Um Mehrdeutigkeit mit dem obigen variadischen Konstruktor zu
    // vermeiden, darf der Typ von xs nicht nach T umwandelbar sein.)
    template <typename X, CH_IF(!CH_CONV(X, T))>
    explicit seq (X xs) : elems() {
	for (auto x : xs) elems.push_back(x);
    }

    // Initialisierung mit den Elementen des Bereichs, der durch die
    // Iteratoren b und e begrenzt wird (mit der in der STL üblichen
    // Bedeutung von b einschließlich bis e ausschließlich).
    // (Um Mehrdeutigkeit mit dem obigen variadischen Konstruktor zu
    // vermeiden, darf der Typ von b und e nicht nach T umwandelbar sein.)
    template <typename I, CH_IF(!CH_CONV(I, T))>
    seq (I b, I e) : elems(b, e) {}

    // Nur für T gleich char:
    // Initialisierung mit einer "C-Zeichenkette",
    // insbesondere mit einem C-String-Literal.
    template <typename X, CH_IF(CH_SAME(X, char) && CH_SAME(T, char))>
    seq (const X* xs) : seq(xs, xs + strlen(xs)) {}

    // Länge der Sequenz, d. h. Anzahl ihrer Elemente.
    int operator* () const {
	return elems.size();
    }

    // Implizite Umwandlung nach bool(like) liefert genau dann true,
    // wenn die Sequenz nicht leer ist.
    operator boollike () const {
	return boollikeval(elems.size() > 0);
    }

    // Verkettung zweier Sequenzen.
    template <typename X, CH_IF(CH_CONV(X, T))>
    seq<T>& operator+= (const seq<X>& s2) {
	elems.insert(elems.end(), s2.elems.begin(), s2.elems.end());
	return *this;
    }
    // Wegen eines Fehlers von g++-10 wird der entsprechende operator+
    // außerhalb des Typs seq definiert.

    // Verkettung einer Sequenz und eines einzelnen Werts x,
    // dessen Typ nach T umwandelbar ist.
    template <typename X, CH_IF(CH_CONV(X, T))>
    seq<T>& operator+= (const X& x) {
	return *this += seq<T>(x);
    }
    template <typename X, CH_IF(CH_CONV(X, T))>
    seq<T> operator+ (const X& x) const {
	return *this + seq<T>(x);
    }
    template <typename X, CH_IF(CH_CONV(X, T))>
    friend seq<T> operator+ (const X& x, const seq<T>& s) {
	return seq<T>(x) + s;
    }

    // Teilsequenz mit Anfangsposition r.first und Endposition r.second.
    template <bool AZ1, bool AZ2>
    seq<T> operator() (CH_range<AZ1, AZ2> r) const {
	int N = elems.size();
	int p = CH_normalize(r.first, N);
	int q = CH_normalize(r.second, N);
	if (p < q) return seq<T>(elems.begin() + p, elems.begin() + q);
	else return seq<T>();
    }

    // Kopie der aktuellen Sequenz, in der die Teilsequenz mit
    // Anfangsposition r.first und Endposition r.second durch die
    // Elemente (von) xx ... ersetzt ist.
    // (Es gibt mindestens ein Argument xx, weil sonst der
    // Klammeroperator mit genau einem Parameter aufgerufen wird.)
    template <bool AZ1, bool AZ2, typename ... XX>
    seq<T> operator() (CH_range<AZ1, AZ2> r, XX ... xx) const {
	return ((*this)(A|r.first) += ... += seq<T>(xx))
						+= (*this)(r.second|Z);
    }
    template <bool AZ, typename ... XX>
    seq<T> operator() (posAZ<AZ> p, XX ... xx) const {
	return ((*this)(A|p) += ... += seq<T>(xx)) += (*this)(p|Z);
    }

    // Element nach bzw. vor Position p, falls vorhanden, sonst nil.
    template <bool AZ>
    T operator[] (posAZ<AZ> p) const {
	int N = elems.size();
	int d = p.factor - 1;
	if (!AZ) d = N - 1 - d;
	if (0 <= d && d < N) return elems[d];
	else return nil;
    }

    // Hilfsfunktion loop für unterschiedliche Resultattypen R der
    // an den Klammeroperator weiter unten übergebenen Funktion f.
    // Der erste Parameter wird nur zur Auswahl der richtigen Variante
    // benötigt.

    // Allgemeines/sonstiges R.
    template <typename R, typename F, typename I, typename ... XX>
    auto loop (R*, F&& f, std::pair<I, I> range, XX&& ... xx) const {
	seq<R> result;
	for (I iter = range.first; iter != range.second; iter++) {
	    R r = std::forward<F>(f)(*iter, std::forward<XX>(xx) ...);
	    result += r;
	}
	return result;
    }

    // R gleich boolA oder boolZ.
    template <bool AZ, typename F, typename I, typename ... XX>
    posAZ<AZ> loop (boolAZ<AZ>*, F&& f,
			    std::pair<I, I> range, XX&& ... xx) const {
	for (I iter = range.first; iter != range.second; iter++) {
	    boolAZ<AZ> r =
		    std::forward<F>(f)(*iter, std::forward<XX>(xx) ...);
	    if (r) return posAZ<AZ>() + 1 + (iter - range.first);
	}
	return posAZ<AZ>();
    }

    // R gleich bool12A oder bool12Z.
    template <bool AZ, typename F, typename I, typename ... XX>
    auto loop (bool12AZ<AZ>*, F&& f, std::pair<I, I> range,
						XX&& ... xx) const {
	seq<T> result;
	std::vector<T>& elems = result.elems;
	for (I iter = range.first; iter != range.second; iter++) {
	    bool12AZ<AZ> r =
		    std::forward<F>(f)(*iter, std::forward<XX>(xx) ...);
	    if (r) {
		elems.insert(AZ ? elems.end() : elems.begin(), *iter);
	    }
	    if (*r) {
		if (**r) {
		    if (AZ) {
			elems.insert(elems.end(), ++iter, range.second);
		    }
		    else {
			elems.insert(elems.begin(), 
				    make_reverse_iterator(range.second),
				    make_reverse_iterator(++iter));
		    }
		}
		break;
	    }
	}
	return result;
    }

    // R gleich bool.
    template <typename F, typename I, typename ... XX>
    auto loop (bool*, F&& f, std::pair<I, I> range, XX&& ... xx) const {
	seq<T> result;
	for (I iter = range.first; iter != range.second; iter++) {
	    bool r = std::forward<F>(f)(*iter, std::forward<XX>(xx) ...);
	    if (r) result += *iter;
	}
	return result;
    }

    // R gleich optional<U>.
    template <typename U, typename F, typename I, typename ... XX>
    auto loop (std::optional<U>*, F&& f, std::pair<I, I> range,
						XX&& ... xx) const {
	seq<U> result;
	for (I iter = range.first; iter != range.second; iter++) {
	    std::optional<U> r =
		std::forward<F>(f)(*iter, std::forward<XX>(xx) ...);
	    if (r) result += r;
	}
	return result;
    }

    // Hilfsfunktion iters für unterschiedliche Resultattypen R
    // der an den Klammeroperator übergebenen Funktion f,
    // die entweder ein Paar normaler STL-Iteratoren
    // oder ein Paar von reverse-Iteratoren liefert.
    template <typename R>
    auto iters (R*) const {
	return std::make_pair(elems.cbegin(), elems.cend());
    }
    auto iters (boolZ*) const {
	return std::make_pair(elems.crbegin(), elems.crend());
    }
    auto iters (bool12Z*) const {
	return std::make_pair(elems.crbegin(), elems.crend());
    }

    // Klammeroperator, der eine Funktion f und beliebige weitere
    // Argumente xx ... erhält.
    template <typename F, typename ... XX>
    auto operator() (F&& f, XX&& ... xx) const {
	// Resultattyp R eines Aufrufs von f.
	using R = decltype(std::forward<F>(f)(std::declval<T>(),
					    std::forward<XX>(xx) ...));

	// Weitergabe der Argumente an die zu R passende Variante von
	// loop mit dem zu R passenden Paar von Iteratoren.
	return loop((R*)nullptr, std::forward<F>(f),
			iters((R*)nullptr), std::forward<XX>(xx) ...);
    }

    // STL-Iterator der Kategorie Random Access,
    // der logisch auf Position p der Sequenz s zeigt.
    template <bool AZ>
    struct iterAZ : std::iterator<std::random_access_iterator_tag, T>,
						CH_opers_t<iterAZ<AZ>> {
	// Da ein Iterator nur solange gültig ist bzw. verwendet werden
	// darf, wie die Sequenz, auf die er sich bezieht, gültig ist,
	// genügt die Speicherung einer Referenz auf die Sequenz.
	// Aus technischen Gründen ist ein Zeiger jedoch einfacher,
	// weil er sowohl einen "default constructor" als auch einen
	// "default assignment operator" besitzt.
	const seq<T>* s;
	posAZ<AZ> p;

	// Konstruktor.
	iterAZ (const seq<T>& s, posAZ<AZ> p) : s(&s), p(p) {}

	// Künstlicher parameterloser Konstruktor, den ein Iterator
	// grundsätzlich haben muss (und der konkret z. B. von
	// std::match_results benötigt wird).
	iterAZ () {}

	// Lesender Zugriff auf das referenzierte Element.
	T operator* () const {
	    return (*s)[p];
	}
	T operator-> () const {
	    return (*s)[p];
	}

	// Lesender Zugriff auf ein anderes Element.
	T operator[] (int n) const {
	    return (*s)[p+n];
	}

	// Addition der ganzen Zahl n.
	iterAZ<AZ>& operator+= (int n) {
	    p += n;
	    return *this;
	}

	// Subtraktion zweier Iteratoren.
	int operator- (iterAZ<AZ> other) const {
	    return p - other.p;
	}

	using CH_opers_t<iterAZ<AZ>>::operator-;

	// Komplement (reverse iterator).
	iterAZ<!AZ> operator~ () const {
	    return iterAZ<!AZ>(*s,
		posAZ<!AZ>() + s->elems.size() + 2 - (p - posAZ<AZ>()));
	}
    };

    // STL-Iterator liefern, der logisch auf Position p zeigt.
    template <bool AZ>
    iterAZ<AZ> operator() (posAZ<AZ> p) const {
	return iterAZ<AZ>(*this, p);
    }

    // STL-Iteratoren für "range-based for loops" liefern.
    iterAZ<true> begin () const {
	return (*this)(A);
    }
    iterAZ<true> end () const {
	return (*this)(A + elems.size());
    }

    // Invertierte Sequenz liefern, die die Elemente der aktuellen
    // Sequenz in umgekehrter Reihenfolge enthält.
    seq<T> operator~ () const {
	return seq<T>((*this)(Z), ~(*this)(A));
    }
};

// Verkettung zweier Sequenzen.
// Wenn man diesen Operator als friend-Funktion innerhalb des Typs seq
// definiert, behauptet g++-10, dass er für seq<double> (aber nicht für
// seq<int>) doppelt definiert sei.
template <typename X, typename Y, typename Z = std::common_type_t<X, Y>>
seq<Z> operator+ (const seq<X>& s1, const seq<Y>& s2) {
    seq<Z> s(s1);
    return s += s2;
}

/*
 *  Deduktionshilfen
 */

// Generische leere Sequenz.
// g++-7 behauptet: cannot deduce template arguments for 'seq' from ().
// g++-10 und clang-7.0.1 haben damit kein Problem.
seq () -> seq<void>;

// Sequenz mit einem oder mehreren Elementen,
// sofern diese einen "gemeinsamen" Typ haben.
template <typename X1, typename ... XX>
seq (X1, XX ...) -> seq<std::common_type_t<X1, XX ...>>;

// Sequenz mit einem Element, dessen Typ selbst ein Sequenztyp seq<X>
// (z. B. str) ist.
// Ohne diese Definition wird offenbar der Kopierkonstruktor von seq<X>
// mit Resultattyp seq<X> statt seq<seq<X>> aufgerufen.
template <typename X>
seq (seq<X>) -> seq<seq<X>>;

// Erkennung von Sequenztypen.
// is_seq_v<T> ist genau dann true, wenn T ein ggf. cv-qualifizierter
// Typ seq<X> mit beliebigem Elementtyp X ist.

template <typename T>
struct CH_is_seq : std::false_type {};

template <typename T>
struct CH_is_seq <seq<T>> : std::true_type {};

template <typename T>
using is_seq = CH_is_seq<std::remove_cv_t<T>>;

template <typename T>
const bool is_seq_v = is_seq<T>::value;

// Elementtyp von Sequenztypen (analog zu std::remove_all_extents).
// Wenn ein T ein ggf. cv-qualifizierter Sequenztyp ist, ist elem_t<T>
// sein direkter oder indirekter Elementtyp (der dann kein Sequenztyp
// mehr ist). Andernfalls ist elem_t<T> gleich T.

template <typename T>
struct CH_elem {
    using type = T;
};

template <typename T>
using elem = CH_elem<std::remove_cv_t<T>>;

template <typename T>
using elem_t = typename elem<T>::type;

template <typename T>
struct CH_elem <seq<T>> {
    using type = elem_t<T>;
};

/*
 *  Zeichenketten
 */

using str = seq<char>;

// Ausgabe der Zeichenkette s auf dem Stream os.
inline std::ostream& operator<< (std::ostream& os, const str& s) {
    for (char c : s) os << c;
    return os;
}

// Zusätzliche Operatoren.

inline str& operator+= (str& s1, const char* s2) {
    return s1 += str(s2);
}

inline str operator+ (const str& s1, const char* s2) {
    return s1 + str(s2);
}
inline str operator+ (const char* s1, const str& s2) {
    return str(s1) + s2;
}

inline seq<str>& operator+= (seq<str>& ss, const char* s) {
    return ss += str(s);
}

inline seq<str> operator+ (const seq<str>& ss, const char* s) {
    return ss + str(s);
}
inline seq<str> operator+ (const char* s, const seq<str>& ss) {
    return str(s) + ss;
}

/*
 *  Vergleichsoperatoren
 */

// Dreiwegvergleich für einen beliebigen Typ T.
template <typename T>
int diff (const T& x1, const T& x2) {
    return x1 - x2;
}

// Dreiwegvergleich für Sequenztypen seq<T>.
template <typename T>
int diff (const seq<T>& s1, const seq<T>& s2) {
    int n1 = *s1, n2 = *s2;
    int n = std::min(n1, n2);
    for (posA p = 1*A; p <= n*A; p++) {
	int d = diff(s1[p], s2[p]);
	if (d != 0) return d;
    }
    return n1 - n2;
}

template <typename T>
bool operator< (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) < 0;
}

template <typename T>
bool operator<= (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) <= 0;
}

template <typename T>
bool operator== (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) == 0;
}

template <typename T>
bool operator!= (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) != 0;
}

template <typename T>
bool operator>= (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) >= 0;
}

template <typename T>
bool operator> (const seq<T>& s1, const seq<T>& s2) {
    return diff(s1, s2) > 0;
}

inline bool operator< (const str& s1, const char* s2) {
    return s1 < str(s2);
}
inline bool operator< (const char* s1, const str& s2) {
    return str(s1) < s2;
}

inline bool operator> (const str& s1, const char* s2) {
    return s1 > str(s2);
}
inline bool operator> (const char* s1, const str& s2) {
    return str(s1) > s2;
}

inline bool operator<= (const str& s1, const char* s2) {
    return s1 <= str(s2);
}
inline bool operator<= (const char* s1, const str& s2) {
    return str(s1) <= s2;
}

inline bool operator>= (const str& s1, const char* s2) {
    return s1 >= str(s2);
}
inline bool operator>= (const char* s1, const str& s2) {
    return str(s1) >= s2;
}

inline bool operator== (const str& s1, const char* s2) {
    return s1 == str(s2);
}
inline bool operator== (const char* s1, const str& s2) {
    return str(s1) == s2;
}

inline bool operator!= (const str& s1, const char* s2) {
    return s1 != str(s2);
}
inline bool operator!= (const char* s1, const str& s2) {
    return str(s1) != s2;
}

/*
 *  Funktionsobjekte zum Suchen und Löschen bestimmter Elemente
 */

// Funktionsobjekt für searchA und searchZ mit Resultattyp boolA
// bzw. boolZ, das genau dann true liefert, wenn das enthaltene
// Funktionsobjekt f zum n-ten Mal true liefert.
template <typename F, bool AZ>
struct CH_searchAZ {
    F f;
    int n;
    CH_searchAZ (const F& f, int n) : f(f), n(n) {}

    template <typename X>
    boolAZ<AZ> operator() (X&& x) /* ohne const! */ {
	return f(std::forward<X>(x)) && --n == 0;
    }
};

// Funktionsobjekte zum Suchen des n-ten Objekts von vorn bzw. hinten,
// das die Eigenschaft f besitzt.
template <typename F>
CH_searchAZ<F, true> searchA (const F& f, int n = 1) {
    return CH_searchAZ<F, true>(f, n);
}
template <typename F>
CH_searchAZ<F, false> searchZ (const F& f, int n = 1) {
    return CH_searchAZ<F, false>(f, n);
}

// Funktionsobjekt zum Löschen aller Objekte mit der Eigenschaft f.
template <typename F>
auto remove (const F& f) {
    return !f;
}

// Funktionsobjekt für removeA und removeZ mit Resultattyp bool12A
// bzw. bool12Z, das zunächst genau dann false liefert, wenn das
// enthaltene Funktionsobjekt f true liefert.
// Wenn f zum n-ten Mal true liefert, wird jedoch bool12A(false, true)
// bzw. bool12Z(false, true) geliefert.
template <bool AZ, typename F>
struct CH_removeAZ {
    F f;
    int n;
    CH_removeAZ (const F& f, int n) : f(f), n(n) {}

    template <typename X>
    bool12AZ<AZ> operator() (X&& x) /* ohne const! */ {
	bool r = f(std::forward<X>(x));
	if (r && --n == 0) return bool12AZ<AZ>(false, true);
	else return !r;
    }
};

// Funktionsobjekte zum Löschen der ersten n Objekte von vorn bzw.
// hinten, die die Eigenschaft f besitzen.
template <typename F>
CH_removeAZ<true, F> removeA (const F& f, int n = 1) {
    return CH_removeAZ<true, F>(f, n);
}
template <typename F>
CH_removeAZ<false, F> removeZ (const F& f, int n = 1) {
    return CH_removeAZ<false, F>(f, n);
}

}

template <>
struct std::hash<CH::str> {
    size_t operator() (const CH::str& s) const {
	size_t h = 0;
	for (char c : s) h = h * 31 + c;
	return h;
    }
};

#endif
