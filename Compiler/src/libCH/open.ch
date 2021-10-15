#line 762 "open/@"
// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#ifndef CH_OPEN_CH
#define CH_OPEN_CH 2021'10'15

#include <algorithm>
#include <functional>
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "meta.ch"
#include "util.ch"
#include "seq.ch"
#include "trace.ch"

namespace CH {

// Durch Definition von CH_GC_OFF kann die automatische
// Speicherbereinigung deaktiviert werden,
// z. B. um die Laufzeit mit und ohne GC zu vergleichen,
// um ein Programm, das keine GC braucht, zu beschleunigen
// oder um bei einem fehlerhaften Programm Fehler in der GC
// (hoffentlich) auszuschließen.
// Im folgenden kann Code, der nur für die Speicherbereinigung
// gebraucht wird, entweder mit #if CH_GC ... #endif
// oder mit CH_GC_CODE(...) geklammert werden.
// Letzteres muss insbesondere innerhalb von Makros verwendet werden.
#ifdef CH_GC_OFF
#undef CH_GC
#define CH_GC_CODE(...)
#else
#define CH_GC 1
#define CH_GC_CODE(...) __VA_ARGS__
#endif

// Hilfstyp mit einem inneren Typ type, der für I gleich 0 ein Synonym
// für void und für andere Werte von I jeweils ein eindeutiger Typ ist.
template <int I>
struct CH_dummy {
    struct type {};
};
template <>
struct CH_dummy<1> {
    using type = void;
};

// Attribut mit dem Namen a allgemein definieren,
// d. h. noch unabhängig von einem konkreten Typ.
// Hierfür wird ein eindeutiger Typ, dessen genauer Name unwichtig ist,
// sowie eine parameterlose Funktion mit dem Namen a definiert, die ein
// Objekt dieses Typs liefert.
// Dieses Objekt besitzt einen maximal generischen Klammeroperator, der
// mit beliebig vielen beliebigen Parametern aufgerufen werden kann,
// die unverändert an die jeweils passende Funktion mit dem Namen
// CH_attr weitergegeben werden; zusätzlich wird als erster Parameter
// das Objekt selbst übergeben.
// Das Makro ATTR definiert beim ersten Aufruf für einen Attributnamen a
// (in einem bestimmten Namensraum einer bestimmten Übersetzungseinheit)
// diese parameterlose Funktion a und den zugehörigen Typ als Ausprägung
// einer Hilfstypschablone mit dem Namen a__.
// Alle weiteren Aufrufe (im selben Namensraum in derselben
// Übersetzungseinheit) sind sozusagen wirkungslos. (Insbesondere
// dürfen sie die Funktion und den Typ nicht erneut definieren.)
// Die Hilfstypschablone a__ mit einem int-Parameter besitzt keine
// allgemeine Definition, damit sie bei jedem Aufruf des Makros erneut
// deklariert werden kann.
// Bei jedem Aufruf des Makros wird eine Spezialisierung mit einem
// jeweils eindeutigen Wert CH_GET_CNT(a) des Parameters erstellt,
// die sowohl die Definition des Klammeroperators als auch die
// Definition der globalen Funktion a als Freundfunktion enthält.
// Formal besitzt diese Freundfunktion einen Parameter mit dem Typ
// CH_dummy<CH_GET_CNT(a)>::type, der beim ersten Aufruf des Makros
// jedoch gleich void ist, sodass die Funktion in diesem Fall faktisch
// parameterlos ist, während bei allen anderen Aufrufen Funktionen mit
// irgendwelchen anderen Parametertypen entstehen.
// Da nur die parameterlose Funktion global deklariert wird, kann
// faktisch auch nur sie verwendet werden.
// Damit Attribute in Definitionsdateien definiert werden können, die
// in mehrere Quelldateien eingebunden werden, ist es essentiell, dass
// der zu einem Attribut a gehörende Typ jedesmal gleich heißt.
// (Sein konkreter Name lautet a__<1>, und er steht auch als a__t zur
// Verfügung.)
// TODO: Namen mit __ sind reserviert.
#define CH_ATTR(a) \
    CH_DEF_CNT(a) \
    template <int> \
    struct a##__; \
    template <> \
    struct a##__ <CH_GET_CNT(a)> { \
	template <typename ... YY> \
	decltype(auto) operator() (YY&& ... yy) const { \
	    return CH_attr(*this, std::forward<YY>(yy) ...); \
	} \
	friend a##__ a (typename CH::CH_dummy<CH_GET_CNT(a)>::type) { \
	    return a##__(); \
	} \
    }; \
    using a##__t = a##__<1>; \
    a##__t a (); \
    CH_INC_CNT(a)

#if CH_GC
// Menge von Nachfolgerfunktionen (siehe unten).
using CH_SuccFunc = void (*) (char* id);
using CH_SuccFuncs = std::set<CH_SuccFunc>;
#endif

// Typ von Objekt-IDs.
using CH_id_t = char*;

// Gemeinsamer Basistyp aller offenen Typen.
// Zum einen kann is_open offene Typen an diesem Basistyp erkennen.
// Zum anderen werden die unten definierten Vergleichsoperatoren
// damit durch "argument-dependent name lookup" gefunden.
struct CH_base {
    // Eindeutige ID eines Objekts.
    CH_id_t id;

    // Normaler Konstruktor, der für flag = true ein leeres Objekt
    // und für flag = false ein nil-Objekt erzeugt.
    CH_base (bool flag) : id(flag ? new char : nullptr) {}

    #if CH_GC

    // Normaler Konstruktor, der zusätzlich die zum Objekt gehörende
    // Menge von Nachfolgerfunktionen erhält.
    CH_base (bool flag, CH_SuccFuncs* succfuncs);

    // Der Kopierkonstruktor darf in diesem Fall nicht verwendet werden.
    CH_base (const CH_base& that) = delete;

    // Statt dessen muss dieser Konstruktor zum Kopieren verwendet
    // werden, damit succfuncs für die Kopie stimmt.
    CH_base (CH_id_t id, CH_SuccFuncs* succfuncs);

    // Die Kopierzuweisung wird implizit korrekt definiert.

    // Verschiebekonstruktor und Verschiebezuweisung werden nicht
    // gebraucht und werden auch nicht implizit definiert, da es
    // (Kopierkonstruktor und) Destruktor explizit gibt.

    // Destruktor.
    ~CH_base ();

    #endif
};

// Erkennung offener Typen.
// is_open_v<T> ist genau dann true, wenn T ein ggf. cv-qualifizierter
// offener Typ ist, der mittels CH_TYPE(T) definiert wurde.
// (Achtung: Für einen offenen Typ T sind is_open_v<T&> und
// is_open_v<const T&> gleich false, weil T& und const T& erst einmal
// Referenztypen sind. Ggf. muss man also erst einmal 
// std::remove_reference_t oder std::decay_t auf einen Typ anwenden,
// bevor man ihn mit is_open_v überprüft.)
// Verwendung von constexpr anstelle von const, weil es in MSVC mit
// const nicht korrekt funktioniert.
template <typename T>
struct CH_is_open {
    static constexpr bool value = std::is_base_of_v<CH::CH_base, T>;
};
template <typename T>
using is_open = CH_is_open<std::remove_cv_t<T>>;
template <typename T>
constexpr bool is_open_v = is_open<T>::value;

// Vergleichsoperatoren.
// operator== kann für einzelne Typen neu definiert werden.
// operator!= ruft wiederum operator== auf, sodass er normalerweise
// nicht umdefiniert werden muss.
template <typename T, CH_IF(is_open_v<T>)>
bool operator== (T x1, T x2) {
    return x1.id == x2.id;
}
template <typename T, CH_IF(is_open_v<T>)>
bool operator!= (T x1, T x2) {
    return !(x1 == x2);
}

// Offenen Typ T, ggf. mit "Schablonenpräfix" temp, definieren.
// temp muss entweder leer oder von der Gestalt template <......> sein.
// Der Konstruktor T () initialisiert sein Objekt als nil-Objekt.
// Der Konstruktor T (uniq) initialisiert sein Objekt als eindeutiges
// leeres echtes Objekt.
// Die Umwandlung nach bool liefert genau dann true, wenn das Objekt
// ein echtes Objekt ist.
// Der Klammeroperator erhält als Parameter ein beliebiges Attribut a
// (genauer die oben erwähnte Funktion a, die zu diesem Attribut gehört)
// sowie beliebige weitere Parameter und gibt diese, zusammen mit dem
// aktuellen Objekt, an den Klammeroperator des Objekts a weiter, der
// sie seinerseits an die passende Funktion mit dem Namen CH_attr
// weitergibt.
// Damit wird ein Aufruf der Art x(a, ......) letztlich auf
// CH_attr(a, x, ......) abgebildet.
// Der letzte Konstruktor initialisiert sein Objekt als eindeutiges
// leeres echtes Objekt und wendet anschließend sozusagen den
// Klammeroperator darauf an.
// (Um unerwünschte Mehrdeutigkeiten zu vermeiden, muss dieser
// Konstruktor mit mindestens zwei Argumenten aufgerufen werden.)
// Der Präfixoperator + liefert ein Duplikat des Objekts.
// Der Indexoperator erhält als Parameter ein beliebiges Attribut a
// und liefert eine logische Referenz auf dieses Attribut des aktuellen
// Objekts. Dabei müssen wie bei CH_write ggf. Funktionen zu den
// entsprechenden Mengen hinzugefügt werden.
// Die Namen der Schablonenparameter beginnen mit __, damit sie nicht
// zufällig mit dem Namen T übereinstimmen.
// TODO: Namen mit __ sind reserviert.
const struct {} uniq;
#define CH_TYPEHELP(T, temp) \
    temp struct T : CH::CH_base { \
	T () : CH::CH_base(false \
			    CH_GC_CODE(, &CH::CH_succfuncs<T>())) {} \
	T (decltype(CH::uniq)) : CH::CH_base(true \
			    CH_GC_CODE(, &CH::CH_succfuncs<T>())) {} \
	CH_GC_CODE( \
	T (const T& CH_that) : CH::CH_base(CH_that.CH::CH_base::id, \
					&CH::CH_succfuncs<T>()) {} \
	) \
	operator bool () const { return CH::CH_base::id; } \
	template <typename __A, typename ... __YY> \
	decltype(auto) operator() (__A a (), __YY&& ... yy) const { \
	    return a()(*this, std::forward<__YY>(yy) ...); \
	} \
	template <typename __A, typename __Y, typename ... __YY> \
	T (__A a (), __Y&& y, __YY&& ... yy) : T(CH::uniq) { \
	    a()(*this, std::forward<__Y>(y), std::forward<__YY>(yy) ...); \
	} \
	T operator+ () const { \
	    if (!*this) return *this; \
	    T dupl = CH::uniq; \
	    for (auto copyfunc : CH::CH_copyfuncs<T>()) { \
		copyfunc(*this, dupl); \
	    } \
	    return dupl; \
	} \
	template <typename __A> \
	auto operator[] (__A a()) const { \
	    using __Y = decltype(a()(*this)); \
	    static auto __dummy = ( \
		CH_GC_CODE( \
		CH::CH_addgcfuncs<__A, T, __Y>(), \
		) \
		CH::CH_copyfuncs<T>().insert( \
					CH::CH_copy<__A, __Y, T>)); \
	    return CH::aref<__Y>(CH::CH_tab<__A, __Y>(), \
						    CH::CH_base::id); \
	} \
    };

// Offenen Typ T definieren.
#define CH_TYPE(T) \
    CH_TYPEHELP(T, )

// Offenen Typ T als Typschablone mit beliebigen
// Schablonenparametern ... definieren.
#define CH_TYPETEMP(T, ...) \
    CH_TYPEHELP(T, CH_LIT(template <__VA_ARGS__>))

#if CH_GC
template <typename A, typename X, typename Y>
void CH_addgcfuncs ();
template <typename Y>
void CH_purge (const Y& x);
#endif

// Wenn ein offener Typ X ein Attribut a mit Zieltyp Y besitzt, enthält
// CH_tab<A, Y>()[x.id] für ein Objekt x des Typs X den Wert dieses
// Attributs für dieses Objekt (sofern es einen solchen Wert gibt).
// A ist hierbei der oben erwähnte Typ a__t, der zum Attribut a gehört.
template <typename A, typename Y>
CH_SIGVAR(CH_LIT(std::unordered_map<CH_id_t, Y>), CH_tab)

// Menge von Attribut-Kopierfunktionen des offenen Typs T.
// Wenn die Schreibfunktion eines Attributs des Typs T zum ersten Mal
// aufgerufen wird oder zum ersten Mal eine logische Referenz auf das
// Attribut erzeugt wird, wird die zugehörge Kopierfunktion zu dieser
// Menge hinzugefügt. (Weil eine Funktion dadurch auch zweimal
// hinzugefügt werden kann, muss eine Menge verwendet werden.)
// Um beim Duplizieren eines Objekts x1 des Typs T alle seine
// Attributwerte in ein anderes Objekt x2 des Typs zu kopieren,
// werden alle Funktionen dieser Menge aufgerufen.
template <typename T>
CH_SIGVAR(std::set<void (*) (T x1, T x2)>, CH_copyfuncs)

// Wert des zum Typ A korrespondierenden Attributs für Objekt x liefern,
// falls vorhanden, andernfalls nil.
// Wenn x gleich nil ist, ist der Resultatwert immer nil.
// Bei Resultatwert nil wird kein Eintrag zur Tabelle hinzugefügt.
// Die Schablonenparameter A und Y müssen bei einem Aufruf explizit
// angegeben werden.
template <typename Y>
Y CH_read (std::unordered_map<CH_id_t, Y>& tab, CH_id_t id) {
    if (tab.count(id)) return tab[id];
    else return nil;
}
template <typename A, typename Y, typename X>
Y CH_read (X x) {
    return CH_read(CH_tab<A, Y>(), x.id);
}

// Wert des zum Typ A korrespondierenden Attributs von Objekt from
// nach Objekt to kopieren, falls vorhanden.
// from und to sind beide nicht nil.
// Der Zieltyp Y muss keinen parameterlosen Konstruktor besitzen.
template <typename A, typename Y, typename X>
void CH_copy (X from, X to) {
    if (CH_tab<A, Y>().count(from.id)) {
	CH_tab<A, Y>().insert_or_assign(to.id, CH_tab<A, Y>()[from.id]);
	#if CH_GC
	CH_purge(CH_tab<A, Y>()[to.id]);
	#endif
    }
}

// Wert des zum Typ A korrespondierenden Attributs für Objekt x
// auf y setzen.
// Wenn x gleich nil ist, ist die Operation wirkungslos.
// Der Zieltyp Y muss keinen parameterlosen Konstruktor besitzen.
// Beim ersten Aufruf dieser Funktion für dieses Attribut wird die
// zuvor definierte Kopierfunktion zur Menge der Kopierfunktionen
// des Typs X hinzugefügt.
template <typename Y>
void CH_write (std::unordered_map<CH_id_t, Y>& tab, CH_id_t id,
							const Y& y) {
    if (id) {
	tab.insert_or_assign(id, y);
	#if CH_GC
	CH_purge(tab[id]);
	#endif
    }
}
template <typename A, typename Y, typename X>
X CH_write (X x, Y y) {
    static auto dummy = (
	#if CH_GC
	CH_addgcfuncs<A, X, Y>(),
	#endif
	CH_copyfuncs<X>().insert(CH_copy<A, Y, X>));
    CH_write(CH_tab<A, Y>(), x.id, y);
    return x;
}

// Einwertiges Attribut a des offenen Typs X mit Zieltyp Y,
// ggf. mit Schablonenpräfix temp, definieren.
// a muss ein einfacher Name sein, X und Y dürfen auch "Typausdrücke"
// wie z. B. std::string oder List<int> sein.
// temp muss entweder leer oder von der Gestalt template <......> sein.
// Es wird eine Lese- und eine Schreibfunktion definiert, die read bzw.
// write geeignet aufruft.
// Die Schreibfunktion hat Resultattyp X und liefert das übergebene
// Objekt x zurück.
// In den Funktionssignaturen wird Y mit wrap_t geklammert, damit es
// z. B. auch void (*) () sein kann (was als Resultattyp direkt nicht
// möglich ist) und damit aus Y keine Schablonenparameter deduziert
// werden, sodass z. B. parent(n, nil) möglich ist, wenn parent eine
// Attributschablone ATTR1TEMP(parent, LIT(Node<P, D>), LIT(Node<P, D>),
// typename P, typename D) ist.
// Damit Attribute in Definitionsdateien definiert werden können, die
// in mehrere Quelldateien eingebunden werden, müssen die Funktionen
// inline definiert werden.
#define CH_ATTR1HELP(a, X, Y, temp) \
    CH_ATTR(a) \
    temp inline CH::wrap_t<Y> CH_attr (a##__t, X x) { \
	return CH::CH_read<a##__t, Y>(x); \
    } \
    temp inline X CH_attr (a##__t, X x, CH::wrap_t<Y> y) { \
	return CH::CH_write<a##__t, Y>(x, y); \
    }

// Einwertiges Attribut ohne Schablonenparameter definieren.
#define CH_ATTR1(a, X, Y) \
    CH_ATTR1HELP(a, X, Y, )

// Einwertiges Attribut mit beliebigen Schablonenparametern ...
// definieren, z. B. ATTR1TEMP(head, List<T>, T, typename T) oder
// ATTR1TEMP(first, CH_LIT(Pair<X, Y>), Y, typename X, typename Y).
#define CH_ATTR1TEMP(a, X, Y, ...) \
    CH_ATTR1HELP(a, CH_LIT(X), CH_LIT(Y), CH_LIT(template<__VA_ARGS__>))

// Mehrwertiges Attribut a des offenen Typs X mit Zieltyp Y, ggf. mit
// Schablonenpräfix temp und Schablonenparametern tpar, definieren.
// temp muss entweder leer oder von der Gestalt template <......> sein,
// tpar muss entsprechend entweder ebenfalls leer sein oder die
// Schablonenparameter ...... und ein abschließendes Komma enthalten.
// Ein mehrwertiges Attribut mit Zieltyp Y entspricht im wesentlichen
// einem einwertigen Attribut mit Zieltyp seq<Y>.
// Insbesondere hat seine Lesefunktion Resultattyp seq<Y>.
// Statt einer einzigen Schreibfunktion zum Ersetzen aller Werte der
// Sequenz auf einmal, gibt es jedoch mehrere Schreibfunktionen mit
// folgender Bedeutung:
// Wenn der zweite Parameter y1 ein einzelner Wert mit Typ Y oder eine
// Sequenz mit Typ seq<Y> ist, werden die aktuellen Werte des Attributs
// durch die Verkettung aller Parameter y1 und yy ... ersetzt, sofern
// die entsprechenden Sequenzverkettungen typkorrekt sind.
// Andernfalls (d. h. wenn der zweite Parameter weder in Y noch in
// seq<Y> umgewandelt werden kann), werden alle Parameterwerte außer
// dem Objekt x an den Klammeroperator der aktuellen Sequenz x(a)
// weitergegeben und die aktuelle Sequenz durch die von diesem Operator 
// zurückgelieferte Sequenz ersetzt (sofern dies typkorrekt ist).
// Achtung: Da Y Kommas enthalten kann, muss es bei der Weitergabe an
// CH_CONV mit CH_LIT geklammert werden.
#define CH_ATTRNHELP(a, X, Y, temp, tpar) \
    CH_ATTR(a) \
    temp inline CH::seq<Y> CH_attr (a##__t, X x) { \
	return CH::CH_read<a##__t, CH::seq<Y>>(x); \
    } \
    template <tpar typename __Y1, typename ... __YY, \
			    CH_IF(CH_CONV(__Y1, CH_LIT(Y)) || \
				CH_CONV(__Y1, CH::seq<CH_LIT(Y)>))> \
    X CH_attr (a##__t, X x, __Y1&& y1, __YY&& ... yy) { \
	return CH::CH_write<a##__t, CH::seq<Y>>(x, \
	    ((CH::seq<Y>() + std::forward<__Y1>(y1)) + ... + \
					std::forward<__YY>(yy))); \
    } \
    template <tpar typename __Y1, typename ... __YY, \
			    CH_IF(!CH_CONV(__Y1, CH_LIT(Y)) && \
				!CH_CONV(__Y1, CH::seq<CH_LIT(Y)>))> \
    X CH_attr (a##__t, X x, __Y1&& y1, __YY&& ... yy) { \
	return CH::CH_write<a##__t, CH::seq<Y>>(x, \
	    x(a)(std::forward<__Y1>(y1), std::forward<__YY>(yy) ...)); \
    }

// Mehrwertiges Attribut ohne Schablonenparameter definieren.
#define CH_ATTRN(a, X, Y) \
    CH_ATTRNHELP(a, CH_LIT(X), CH_LIT(Y), , )

// Mehrwertiges Attribut mit beliebigen Schablonenparametern ...
// definieren.
#define CH_ATTRNTEMP(a, X, Y, ...) \
    CH_ATTRNHELP(a, CH_LIT(X), CH_LIT(Y), \
	CH_LIT(template<__VA_ARGS__>), CH_LIT(__VA_ARGS__, ))

// Funktionssignatur mit Parametern ...
// für ein virtuelles Attribut a liefern.
#define CH_FUNC(a, ...) \
    CH_attr (a##__t, __VA_ARGS__)

// Hilfstyp mit Streuwertfunktion für den offenen Typ T,
// sodass std::hash<T> nur von hash<T> abgeleitet werden muss.
template <typename T>
struct hash {
    size_t operator() (T x) const {
	return std::hash<CH_id_t>()(x.id);
    }
};

// Makro, das genau diese Ableitung für den Typ T macht
// (und deshalb im globalen Namensraum verwendet werden muss).
#define CH_HASH(T) \
    template <> \
    struct std::hash<T> : CH::hash<T> {};
#define CH_HASHTEMP(T, ...) \
    template <__VA_ARGS__> \
    struct std::hash<T> : CH::hash<T> {};

// Logische Referenz auf den Attributwert eines Objekts.

// Basisklasse für alle Typen Y.
template <typename Y>
struct CH_aref_base {
    // ID des Objekts (oder nullptr).
    CH_id_t id;

    // Wertetabelle des Attributs.
    std::unordered_map<CH_id_t, Y>& tab;

    // Konstruktor.
    CH_aref_base (std::unordered_map<CH_id_t, Y>& tab, CH_id_t id)
						: tab(tab), id(id) {}

    // Besitzt das Objekt einen Wert für das Attribut
    // (der auch nil sein kann)?
    operator bool () const {
	return tab.count(id);
    }

    // Wert des Attributs entfernen.
    void operator~ () const {
	tab.erase(id);
    }

    // Wert des Attributs liefern.
    Y operator() () const {
	return CH_read(tab, id);
    }

    // Wert des Attributs auf y setzen.
    void operator() (const Y& y) const {
	CH_write(tab, id, y);
    }
};

// Eigentliche Klasse für alle Typen Y außer Sequenztypen.
template <typename Y>
struct aref : CH_aref_base<Y> {
    using CH_aref_base<Y>::CH_aref_base;
};

// Eigentliche Klasse für alle Sequenztypen Y = seq<Z>.
template <typename Z>
struct aref <seq<Z>> : CH_aref_base<seq<Z>> {
    using Y = seq<Z>;
    using base = CH_aref_base<Y>;
    using base::base;
    using base::operator();

    // Wert des Attributs ändern, sofern es sequenzwertig ist.
    // Diese zusätzlichen Klammeroperatoren können nicht mit
    // zusätzlicher Bedingung is_seq<Y> in die Basisklasse integriert
    // werden (sodass man auf die abgeleiteten Klassen verzichten
    // könnte), weil dort der Elementtyp Z nicht verfügar ist.
    // (Y::elem_t geht nicht, weil es für Typen Y, die keine
    // Sequenztypen sind, zu Übersetzungsfehlern führt!)
    template <typename Y1, typename ... YY,
			    CH_IF(CH_CONV(Y1, Z) || CH_CONV(Y1, Y))>
    void operator() (Y1&& y1, YY&& ... yy) const {
	CH_write(base::tab, base::id, ((Y() + std::forward<Y1>(y1)) + ... +
					    std::forward<YY>(yy)));
    }
    template <typename Y1, typename ... YY,
			    CH_IF(!CH_CONV(Y1, Z) && !CH_CONV(Y1, Y))>
    void operator() (Y1&& y1, YY&& ... yy) const {
	Y y = CH_read(base::tab, base::id);
	CH_write(base::tab, base::id,
		    y(std::forward<Y1>(y1), std::forward<YY>(yy) ...));
    }
};

// Automatische Speicherbereinigung.
#if CH_GC

// Menge von Nachfolgerfunktionen des offenen Typs T
// analog zur Menge der Kopierfunktionen (siehe oben).
// In der Markierungsphase werden alle Funktionen dieser Menge für
// jedes erreichbare Objekt aufgerufen, um die Nachfolger dieses
// Objekts zu markieren.
template <typename T>
CH_SIGVAR(CH_SuccFuncs, CH_succfuncs)

// Menge von Reinigungsfunktionen, die für jede Attributtabelle eine
// Funktion enthält, die in der Reinigungsphase alle Einträge von
// "toten" Objekten aus dieser Tabelle entfernt.
// (Auch hier muss eine Menge verwendet werden, weil eine Funktion
// prinzipiell mehrmals hinzugefügt werden kann.)
CH_SIGVAR(std::set<void (*) ()>, CH_sweepfuncs)

// Menge aller Objekt-IDs.
// (Obwohl es logisch eine Menge ist, kann ein vector verwendet werden,
// weil keine ID mehrmals hinzugefügt wird.)
CH_SIGVAR(std::vector<CH_id_t>, CH_allids)

// Wurzelmenge, die die Adressen aller Wurzelobjekte und Zeiger auf die
// zugehörigen Mengen von Nachfolgerfunktionen enthält.
struct CH_base;
CH_SIGVAR(CH_LIT(std::unordered_map<const CH_base*, CH_SuccFuncs*>),
							    CH_roots)

// Markierung erreichbarer Objekte.
// Damit man die Markierungen am Ende einer Speicherbereinigung nicht
// explizit entfernen muss, wird der bool-Wert am Ende jeder
// Speicherbereinigung invertiert.
// Der Konstruktor von CH_base "markiert" neue Objekte mit !CH_marked.
inline bool CH_marked = true;

// Angestrebter Belegungsfaktor des "Heaps", d. h. Anteil der Objekte,
// die eine Speicherbereinigung überleben sollen.
// Kann durch Definition von CH_GC_OCC verändert werden.
#ifndef CH_GC_OCC
#define CH_GC_OCC 0.4
#endif
const double CH_occ = CH_GC_OCC;

// Minimale und momente Größe des "Heaps", d. h. Anzahl verschiedener
// Objekte (IDs), die gleichzeitig existieren können.
// Wenn die Größe von CH_allids den Wert CH_cap erreicht hat, wird eine
// Speicherbereinigung durchgeführt.
// Anschließend wird CH_cap auf die neue Größe von CH_allids().size()
// geteilt durch CH_cap gesetzt, sodass der angestrebte Belegungsfaktor
// in diesem Moment exakt erreicht wird.
// Der Anfangswert ist relativ beliebig.
const int CH_mincap = 1000;
inline int CH_cap = CH_mincap;

// follow muss für ein Objekt y eines beliebigen Typs Y die
// Funktion f für jeden Nachfolger von y aufrufen, d. h. für
// jedes (logisch) in y enthaltene Objekt eines offenen Typs
// (ggf. einschließlich y selbst).
// Diese Nachfolger müssen per Referenz an f übergeben werden,
// damit bei Bedarf ihre Adressen ermittelt werden können.
// follow wird einerseits in der Markierungsphase verwendet,
// um die Nachfolger eines Objekts zu markieren.
// Andererseits wird es auch nach dem Schreiben eines Werts in einer
// Attributtabelle für das Zielobjekt y aufgerufen (CH_purge).
// Weil dabei für jedes in y enthaltene Objekt eines offenen Typs
// ein Konstruktor von CH_base aufgerufen wurde, der das Zielobjekt
// fälschlicherweise zur Wurzelmenge der Speicherbereinigung
// hinzugefügt hat, muss die Funktion f für jedes solche Objekt
// aufgerufen, um dies wieder rückgängig zu machen.

// Für einen offenen Typ Y wird f(y) aufgerufen.
template <typename Y, typename F, CH_IF(is_open_v<Y>)>
void follow (const Y& y, const F& f) {
    f(y);
}

// Für eine ein- oder mehrfache Sequenz, deren Elementtyp ein offener
// Typ ist, wird follow rekursiv für jedes Element der Sequenz
// aufgerufen.
// Die explizite Überprüfung, ob der Elementtyp ein offener Typ ist,
// vermeidet unnötige Aufrufe für Elemente mit anderen Typen.
template <typename Y, typename F,
			CH_IF(is_seq_v<Y> && is_open_v<elem_t<Y>>)>
void follow (const Y& s, const F& f) {
    // Um an die Adressen der Elemente zu kommen,
    // muss man direkt s.elems verwenden.
    for (const auto& x : s.elems) follow(x, f);
}

// Für sonstige Typen Y wird nichts gemacht.
// Eine variadische Schablone ist besser als eine Funktion mit Ellipse
// in der Parameterliste und wird bei der Auflösung von Überladungen
// trotzdem nur berücksichtigt, wenn keine normale Funktion oder
// Schablone passt.
// Wenn die Parameterliste nur aus einer Ellipse besteht,
// meckert zumindest clang++-7:
// cannot pass object of non-trivial type through variadic function;
// call will abort at runtime
// Eine Ellipse nach einem expliziten ersten Parametertyp Y hat er
// akzeptiert.
// Aber wenn man ein Funktionsobjekt übergeben würde, dessen Klasse F
// z. B. einen nicht-trivialen Kopierkonstruktor hat, würde das wohl
// auch nicht gehen.
template <typename ... TT>
void follow (const TT& ...) {}

// Die folgende Definition für sonstige Typen hat den Nachteil, dass
// sie wirklich alles erfasst, was von den ersten beiden Definitionen
// nicht erfasst wird, und deshalb keinen Raum für eventuelle weitere
// (partielle) Spezialisierungen lässt.
#if 0
// Beachte: is_open_v<elem_t<Y>> =
// is_open_v<Y> || is_seq_v<Y> && is_open_v<elem_t<Y>>
// Damit erfasst !is_open_v<elem_t<Y>> genau diejenigen Typen,
// die von den ersten beiden Definitionen nicht erfasst werden.
template <typename Y, typename F, CH_IF(!is_open_v<elem_t<Y>>)>
void follow (const Y& y, const F& f) {}
#endif

// Der Anwender kann bei Bedarf weitere Spezialisierungen definieren,
// z. B. für Standard-Containertypen.

// Warteschlange für die Breitensuche der Markierungsphase,
// die Objekt-IDs und Zeiger auf die zugehörigen Mengen von
// Nachfolgerfunktionen enthält.
using CH_pair = std::pair<CH_id_t, CH_SuccFuncs*>;
CH_SIGVAR(std::queue<CH_pair>, CH_queue);

// Objekt mit der ID id markieren, sofern nötig.
// Resultatwert true genau dann, wenn das Objekt markiert wurde.
inline bool CH_mark (CH_id_t id) {
    if (id && *id != CH_marked) {
	*id = CH_marked;
	return true;
    }
    return false;
}

// Alle Nachfolger des Objekts mit ID id, die sich (logisch) im
// Attribut A dieses Objekts befinden, markieren und zur Warteschlange
// hinzufügen, sofern sie noch nicht markiert sind.
// Außerdem alle diese Nachfolger (egal, ob markiert oder nicht) ggf.
// aus der Wurzelmenge entfernen, in der sie sich fälschlicherweise
// befinden könnten.
template <typename A, typename Y>
void CH_succ (CH_id_t id) {
    if (CH_tab<A, Y>().count(id)) {
	follow(CH_tab<A, Y>()[id],
	    [] (auto& z) {
		CH_id_t id = z.id;
		if (CH_mark(id)) {
		    using Z = std::decay_t<decltype(z)>;
		    CH_queue().push(CH_pair(id, &CH_succfuncs<Z>()));
		}
		CH_roots().erase(&z);
	    });
    }
}

// Alle Nachfolger des Objekts y aus der Wurzelmenge entfernen.
template <typename Y>
void CH_purge (const Y& y) {
    follow(y, [] (auto& z) { CH_roots().erase(&z); });
}

// Reinigungsfunktion des Attributs A, die aus der entsprechenden
// Attributtabelle alle Einträge entfernt, deren Schlüssel die ID
// eines nicht markierten Objekts ist.
template <typename A, typename Y>
void CH_sweep () {
    auto i = CH_tab<A, Y>().begin(), e = CH_tab<A, Y>().end();
    while (i != e) {
	CH_id_t id = i->first;
	if (*id != CH_marked) i = CH_tab<A, Y>().erase(i);
	else i++;
    }
}

// Nachfolger- und Reinigungsfunktion des Attributs A
// zur jeweiligen Menge hinzufügen.
template <typename A, typename X, typename Y>
void CH_addgcfuncs () {
    CH_succfuncs<X>().insert(CH_succ<A, Y>);
    CH_sweepfuncs().insert(CH_sweep<A, Y>);
}

// Speicherbereinigung durchführen.
inline void CH_gc () {
    // Markierungsphase.

    // Wurzelobjekte markieren und zur Warteschlange hinzufügen.
    for (auto root : CH_roots()) {
	CH_id_t id = root.first->id;
	if (CH_mark(id)) CH_queue().push(CH_pair(id, root.second));
    }

    // Warteschlange abarbeiten, d. h. für jedes Objekt seine
    // Nachfolgerfunktionen aufrufen, die seine Nachfolger markieren
    // und zur Warteschlange hinzufügen.
    while (!CH_queue().empty()) {
	CH_pair pair = CH_queue().front();
	CH_queue().pop();
	for (CH_SuccFunc succfunc : *pair.second) succfunc(pair.first);
    }

    // Reinigungsphase.

    // Die Reinigungsfunktionen aller Attributtabellen aufrufen,
    // um die Einträge aller nicht markierten Objekte zu entfernen.
    for (auto sweepfunc : CH_sweepfuncs()) sweepfunc();

    // Anschließend (!) die char-Objekte freigeben, auf die die IDs
    // der nicht mehr erreichbaren Objekte zeigen, und diese IDs aus
    // CH_allids entfernen.
    std::vector<CH_id_t> ids;
    for (CH_id_t id : CH_allids()) {
	if (*id == CH_marked) ids.push_back(id);
	else delete id;
    }
    CH_TRACE_BLOCK("CH_gc: ", CH_allids().size(), " -> ", ids.size())
    std::swap(CH_allids(), ids);

    // Globale Variablen anpassen.
    CH_marked = !CH_marked;
    CH_cap = std::max(int(CH_allids().size() / CH_occ), CH_mincap);
}

// Konstruktoren und Destruktor von CH_base.

// Normaler Konstruktor.
inline CH_base::CH_base (bool flag, CH_SuccFuncs* succfuncs)
						    : CH_base(flag) {
    // Ggf. eine Speicherbereinigung durchführen.
    if (CH_allids().size() == CH_cap) CH_gc();

    // Die Markierung des Objekts wird in dem char-Objekt gespeichert,
    // auf das id zeigt.
    // Da CH_gc den Wert von CH_marked umdreht, darf *id erst danach
    // initialisiert werden, damit das neue Objekt nicht bereits
    // fälschlicherweise markiert ist (was an und für sich nicht so
    // schlimm wäre, aber dazu führt, dass dann seine Nachfolger nicht
    // mehr markiert werden, was fatal wäre).
    CH_roots()[this] = succfuncs;
    if (id) {
	*id = !CH_marked;
	CH_allids().push_back(id);
    }
}

// Ersatz für den Kopierkonstruktor.
inline CH_base::CH_base (CH_id_t id, CH_SuccFuncs* succfuncs) : id(id) {
    CH_roots()[this] = succfuncs;
}

// Destruktor.
inline CH_base::~CH_base () {
    CH_roots().erase(this);
}

#endif

}

#ifdef USING_CH
#define TYPE CH_TYPE
#define TYPETEMP CH_TYPETEMP
#define ATTR CH_ATTR
#define ATTR1 CH_ATTR1
#define ATTR1TEMP CH_ATTR1TEMP
#define ATTRN CH_ATTRN
#define ATTRNTEMP CH_ATTRNTEMP
#define FUNC CH_FUNC
#define HASH CH_HASH
#define HASHTEMP CH_HASHTEMP
#endif

#endif
