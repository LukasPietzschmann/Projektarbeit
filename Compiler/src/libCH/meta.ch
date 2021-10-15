// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef CH_META_CH
#define CH_META_CH 2021'03'24

#include <type_traits>

namespace CH {

// Size<i> ist Untertyp von Size<i-1> und hat Größe i.
template <int i>
struct CH_Size : CH_Size<i-1> {
    char x;
};

// Size<1> hat Größe 1.
template <>
struct CH_Size<1> {
    char x;
};

// Prinzipiell beliebiger Maximalwert eines Zählers.
const int CH_N = 64;

// Zähler und Hilfszähler mit dem Namen name definieren
// und mit Wert 1 initialisieren.
// Wenn bereits ein Zähler mit diesem Namen (im selben Namensraum)
// definiert wurde, ist die Anweisung wirkungslos (und insbesondere
// nicht fehlerhaft).
#define CH_DEF_CNT(name) \
    CH::CH_Size<1> name##__1 (CH::CH_Size<1>); \
    CH::CH_Size<1> name##__2 (CH::CH_Size<1>); \
    CH::CH_Size<1> name##__3 (CH::CH_Size<1>); \
    CH::CH_Size<1> name##__4 (CH::CH_Size<1>); \
    CH::CH_Size<1> name##__5 (CH::CH_Size<1>);

// Zähler und Hilfszähler mit dem Namen name abfragen.
#define CH_GET_CNT(name) \
    CH_GET_CNT1(name)
#define CH_GET_CNT1(name) \
    sizeof name##__1(CH::CH_Size<CH::CH_N * CH_GET_CNT2(name)>())
#define CH_GET_CNT2(name) \
    sizeof name##__2(CH::CH_Size<CH::CH_N * CH_GET_CNT3(name)>())
#define CH_GET_CNT3(name) \
    sizeof name##__3(CH::CH_Size<CH::CH_N * CH_GET_CNT4(name)>())
#define CH_GET_CNT4(name) \
    sizeof name##__4(CH::CH_Size<CH::CH_N * CH_GET_CNT5(name)>())
#define CH_GET_CNT5(name) \
    sizeof name##__5(CH::CH_Size<CH::CH_N>())

// Zähler mit dem Namen name um 1 erhöhen.
// Wenn der neue Wert ein Vielfaches von N bzw. N*N bzw. ... ist,
// wird auch der Hilfszähler mit der Nummer 2 bzw. 3 bzw. ...
// um 1 erhöht.
// Wenn man die Reihenfolge der Deklarationen umkehren würde,
// würden die Hilfszähler jeweils einen Schritt später erhöht werden,
// was aber genauso funktionieren würde.
#define CH_INC_CNT(name) \
    CH::CH_Size<CH_GET_CNT(name)+1> name##__1 \
	(CH::CH_Size<CH_GET_CNT(name)+1>); \
    CH::CH_Size<CH_GET_CNT(name)/CH::CH_N+1> name##__2 \
	(CH::CH_Size<CH_GET_CNT(name)/CH::CH_N+1>); \
    CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N+1> name##__3 \
	(CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N+1>); \
    CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N/CH::CH_N+1> name##__4 \
	(CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N/CH::CH_N+1>); \
    CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N/CH::CH_N/CH::CH_N+1> name##__5 \
	(CH::CH_Size<CH_GET_CNT(name)/CH::CH_N/CH::CH_N/CH::CH_N/CH::CH_N+1>);

#ifdef USING_CH
#define DEF_CNT CH_DEF_CNT
#define GET_CNT CH_GET_CNT
#define INC_CNT CH_INC_CNT
#endif

// Hilfstyp für CH_IF.
template <bool ... B>
struct conjunction {
    static const int value = (B && ...);
};

// IF(x) kann am Ende einer Schablonenparameterliste verwendet werden,
// damit die zugehörige Schablone nur verwendet werden kann, wenn die
// Bedingungen ... zur Übersetzungszeit erfüllt sind.
// Der Typ int und der Wert 0 sind willkürlich, aber sie müssen
// zusammenpassen.
#define CH_IF(...) \
    std::enable_if_t<CH::conjunction<__VA_ARGS__>::value, int> = 0

// SAME(X, Y) und CONV(X, Y) kann innerhalb von IF(......) verwendet
// werden, um auszudrücken, dass die Typen X und Y gleich sein müssen
// bzw. dass X implizit in Y umwandelbar sein muss.
#define CH_SAME(X, Y) std::is_same_v<X, Y>
#define CH_CONV(X, Y) std::is_convertible_v<X, Y>

#ifdef USING_CH
#define IF CH_IF
#define SAME CH_SAME
#define CONV CH_CONV
#endif

}

#endif
