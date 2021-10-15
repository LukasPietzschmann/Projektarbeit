// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef CH_UTIL_CH
#define CH_UTIL_CH 2021'08'19

#include <type_traits>

namespace CH {

// Nil-Wert des Typs T.
template <typename T>
const T nil_v = T();

// Generischer Nilwert,
// der implizit in jeden Typ T umgewandelt werden kann.
const struct {
    template <typename T>
    operator T () const {
	return nil_v<T>;
    }
} nil;

// bool-artiger Typ.
using boollike = void*;

// Umwandlung von bool nach boollike.
inline boollike boollikeval (bool b) {
    static char dummy;
    return b ? &dummy : nullptr;
}

// Dreiwertige Logik.
struct bool3 {
    // 0/1/2 bedeutet false/maybe/true.
    int value;

    // Interner Konstruktor.
    explicit bool3 (int value) : value(value) {}

    // Implizite Umwandlung von und nach bool.
    bool3 (bool value = false) : value(2 * value) {}
    operator bool () const { return value == 2; }
};

// Konstante maybe.
const bool3 maybe(1);

// Vergleichsoperatoren.
// Damit Anwendungen mit "gemischten" Operanden (ein Operand hat Typ
// bool3, der andere einen Typ, der in bool umgewandelt werden kann)
// eindeutig sind, sind pro Operator drei Definitionen nötig.
#define AUX(op) \
    inline bool operator op (bool3 x, bool3 y) { return x.value op y.value; } \
    inline bool operator op (bool3 x, bool y) { return x op bool3(y); } \
    inline bool operator op (bool x, bool3 y) { return bool3(x) op y; }
AUX(<)
AUX(<=)
AUX(==)
AUX(!=)
AUX(>=)
AUX(>)
#undef AUX

// Negation.
inline bool3 operator~ (bool3 x) { return bool3(2 - x.value); }
inline bool3 operator! (bool3 x) { return bool3(2 - x.value); }

// Konjunktion und Disjunktion.
inline bool3 operator&= (bool3& x, bool3 y) {
    if (y.value < x.value) x.value = y.value;
    return x;
}
inline bool3 operator|= (bool3& x, bool3 y) {
    if (y.value > x.value) x.value = y.value;
    return x;
}
#define AUX(op, opeq) \
    inline bool3 operator op (bool3 x, bool3 y) { return x opeq y; } \
    inline bool3 operator op (bool3 x, bool y) { return x op bool3(y); } \
    inline bool3 operator op (bool x, bool3 y) { return bool3(x) op y; }
AUX(&, &=)
AUX(|, |=)
AUX(&&, &=)
AUX(||, |=)
#undef AUX

// Hilfsmakro zur Klammerung von Text, der Kommas enthalten kann,
// aber trotzdem als ein einziges Makroargument übergeben werden soll.
#define CH_LIT(...) __VA_ARGS__

// Globale bzw. Thread-lokale Variable name mit Typ T und optionaler
// Initialisierung ... definieren, die garantiert bei ihrer ersten
// Verwendung initialisiert ist.
#define CH_SIGVAR(T, name, ...) \
    inline std::add_lvalue_reference_t<T> name () { \
	static T name { __VA_ARGS__ }; \
	return name; \
    }
#define CH_SITVAR(T, name, ...) \
    inline std::add_lvalue_reference_t<T> name () { \
	thread_local T name { __VA_ARGS__ }; \
	return name; \
    }

// Die identische Typfunktion.
template <typename T>
struct wrap {
    using type = T;
};
template <typename T>
using wrap_t = typename wrap<T>::type;

#ifdef USING_CH
#define LIT CH_LIT
#define SIGVAR CH_SIGVAR
#define SITVAR CH_SITVAR
#endif

}

#endif
