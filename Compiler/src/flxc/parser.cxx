// Copyright (C) 2021 Prof. Dr. Christian Heinlein

#include <iostream>
#include <queue>
#include <memory>
#include <utility>
#include <unordered_set>
using namespace std;

#include "trace.ch"
#define TRBL CH_TRACE_BLOCK
#define TRLN CH_TRACE_LINE

#include "scanner.h"
#include "parser.h"

// Für die Ablaufverfolgung bekommt jeder Operator eine eindeutige
// Nummer, die ihm bei seiner ersten Ausgabe zugewiesen wird.
// Diese erste Ausgabe eines Operators erfolgt in der von parse
// aufgerufenen Funktion initial. Dadurch stimmt die Nummer eines
// Operators mit seiner Position in der Sequenz opers (gezählt ab 1)
// von parse überein.
int next_number = 1;
ATTR1(number_, Oper, int)

// Die Ausgabe eines Operators besteht nur aus dieser Nummer.
ostream& operator<< (ostream& os, Oper oper) {
    if (!oper(number_)) oper(number_, next_number++);
    return os << oper(number_);
}

// Die Ausgabe einer Sequenz von Operatoren oder eines Katalogs
// besteht aus den Nummern der enthaltenen Operatoren.
ostream& operator<< (ostream& os, seq<Oper> opers) {
    str sep = "";
    for (Oper oper : opers) {
	os << sep << oper;
	sep = ",";
    }
    return os;
}
ostream& operator<< (ostream& os, Cat cat) {
    return os << cat(opers_);
}

// Die Ausgabe einer Position pos ist die Differenz pos - A.
ostream& operator<< (ostream& os, posA pos) {
    return os << pos - A;
}

// Die Ausgabe eines Ausdrucks besteht aus Anfangsposition,
// Nummer des Operators und Endposition.
ostream& operator<< (ostream& os, Expr expr) {
    return os << expr(beg_) << "[" << expr(oper_) << "]" << expr(end_);
}

// Rekursiv alle atomaren Einträge der Reihe row durchlaufen
// (vorwärts für posX gleich posA, rückwärts für posX gleich posZ)
// und für jeden Eintrag die Funktion f aufrufen.
// Abbruch, wenn ihr Resultatwert nicht nil ist.
// Der Resultatwert von traverse ist dann dieser Resultatwert von f.
// Andernfalls ist der Resultatwert nil.
template <typename posX, typename F>
auto traverse (const Row& row, const F& f) -> decltype(f(Item())) {
    const posX X = posX() + 1;

    // Einträge item der Reihe row
    // in der richtigen Reihenfolge durchlaufen.
    for (int i = 1, m = *row; i <= m; i++) {
	Item item = row[i*X];

	// Wenn der Eintrag atomar ist:
	if (item(word_) || item(opnd_)) {
	    // Funktion f mit dem Eintrag aufrufen.
	    // Ende, wenn ihr Resultatwert nicht nil ist.
	    if (auto result = f(item)) return result;
	}
	// Andernfalls ist der Eintrag eine Klammer:
	else {
	    // Die "echten" Durchläufe pass der Klammer
	    // in der gleichen Reihenfolge durchlaufen.
	    for (int j = 1, n = *item(passes_); j <= n; j++) {
		Pass pass = item(passes_)[j*X];
		if (!pass(choice_)) break;

		// Rekursiver Aufruf für den Zweig des aktuellen
		// Durchlaufs.
		// Ende, wenn sein Resultatwert nicht nil ist.
		if (auto result = traverse<posX>(pass(branch_), f)) {
		    return result;
		}
	    }
	}
    }

    return nil;
}

template <typename F>
auto traverseA (const Row& row, const F& f) {
    return traverse<posA>(row, f);
}

template <typename F>
auto traverseZ (const Row& row, const F& f) {
    return traverse<posZ>(row, f);
}

// Den zum Parameter par korrespondierenden Eintrag in der zur Signatur
// sig korrespondierenden Reihe row liefern.
Item search (Par par, Sig sig, Row row) {
    // Wenn par gleich nil ist, ist nichts zu tun.
    if (!par) return nil;

    // Die Teile part der Signatur sig und
    // die korrespondierenden Einträge item der Reihe row durchlaufen.
    for (int i = 1, n = *sig; i <= n; i++) {
	Part part = sig[i*A];
	Item item = row[i*A];

	// Wenn part ein Parameter gleich par ist, item liefern.
	// (Da par gleich nil oben bereits abgefangen wird, wird auch
	// part(par_) gleich nil hier korrekt behandelt.)
	if (part(par_) == par) return item;

	// Wenn part eine Klammer ist, wird ihr letzter Durchlauf
	// rekursiv durchsucht.
	if (Pass pass = item(passes_)[Z]) {
	    if (item = search(par, part(alts_)[pass(choice_)],
						    pass(branch_))) {
		return item;
	    }
	}
    }

    return nil;
}

// Den zum Parameter par korrespondierenden Eintrag des Ausdrucks expr
// liefern.
Item search (Par par, Expr expr) {
    return search(par, expr(oper_)(sig_), expr(row_));
}

// Aktuelles Signaturteil und aktueller Eintrag eines Ausdrucks.
ATTR1(currpart_, Expr, Part)
ATTR1(curritem_, Expr, Item)

// Befindet sich ein Signaturteil garantiert, eventuell oder
// garantiert nicht ganz am Ende eines zugehörgen Ausdrucks?
ATTR1(back_, Part, bool3)

// Attribut back rekursiv für alle Teile der Signatur sig setzen
// (sofern nötig).
void setback (const Sig& sig, bool3 back) {
    // Teile part der Signatur sig von hinten nach vorn durchlaufen.
    for (Part part : ~sig) {
	// Attribut back von part auf den aktuellen Wert von back
	// setzen.
	part(back_, back);

	// Wenn part eine Klammer ist:
	// Alternativen der Klammer rekursiv durchlaufen.
	for (Sig alt : part(alts_)) {
	    // Wenn die Klammer wiederholbar ist, können sich die
	    // Teile ihrer Alternativen (und auch die Teile vor der
	    // Klammer) nicht mehr garantiert am rechten Rand befinden.
	    if (part(rep_)) back = maybe;
	    setback(alt, back);
	}

	// Wenn part optional ist, kann sich das Teil davor noch
	// eventuell am rechten Rand befinden, andernfalls garantiert
	// nicht mehr.
	if (part(opt_)) back = maybe;
	else return;
    }
}

// Aktuelle Stelle des Ausdrucks expr einen Schritt nach rechts (wenn
// up gleich false) bzw. einen Schritt nach oben (wenn up gleich true)
// bewegen, falls möglich.
bool move (Expr expr, bool up) {
    // Beginne mit der Gesamtsignatur und der Hauptreihe von expr.
    Sig sig = expr(oper_)(sig_);
    Row row = expr(row_);

    // Signaturteil und Eintrag an der momentanen bzw. vorigen Stelle.
    Part part, parentpart;
    Item item, parentitem;

    while (true) {
	// Signaturteil und Eintrag der momentanen Stelle.
	int i = *row;
	part = sig[i*A];
	item = row[i*A];

	// Abbruch der Schleife, wenn das die aktuelle Stelle ist.
	if (part == expr(currpart_)) break;

	// Anonsten einen Schritt weitergehen.
	parentpart = part;
	parentitem = item;
	Pass pass = item(passes_)[Z];
	sig = part(alts_)[pass(choice_)];
	row = pass(branch_);
    }

    if (up) {
	// Bewegung nach oben.
	if (!parentpart) return false;
	part = parentpart;
	item = parentitem;
    }
    else {
	// Bewegung nach rechts.
	int i = *row + 1;
	if (i > *sig) return false;
	part = sig[i*A];

	// Neuen Eintrag zur übergeordneten Reihe hinzufügen.
	item = uniq;
	if (parentitem) parentitem(passes_)[Z](branch_, Z, item);
	else expr(row_, Z, item);
    }

    // currpart und curritem aktualisieren.
    expr(currpart_, part)(curritem_, item);
    return true;
}

// Aktuelle Stelle des Ausdrucks expr einen Schritt nach rechts bewegen,
// falls möglich.
bool right (Expr expr) {
    return move(expr, false);
}

// Aktuelle Stelle des Ausdrucks expr einen Schritt nach oben bewegen,
// falls möglich.
bool up (Expr expr) {
    return move(expr, true);
}

// Aktuelle Stelle des Ausdrucks expr einen Schritt nach unten
// zur Alternative alt bewegen.
void down (Expr expr, posA alt) {
    Sig sig = expr(currpart_)(alts_)[alt];
    Part part = sig[A];
    Item item = uniq;
    expr(curritem_)(passes_, Z, Pass(choice_, alt)(branch_, item));
    expr(currpart_, part)(curritem_, item);
}

// Duplikat des Ausdrucks expr erzeugen und liefern, bei dem der
// gesamte Pfad zum aktuellen Eintrag ebenfalls dupliziert ist.
Expr dupl (Expr expr) {
    // Flache Kopie von expr.
    Expr dupl = +expr;

    // Letzten Eintrag der Hauptreihe kopieren.
    Item last = dupl(row_)[Z];
    Item item = +last;
    dupl(row_, 1|Z, item);

    // Solange dieser Eintrag nicht der aktuelle Eintrag ist,
    // muss er eine Klammer sein:
    Item curritem = dupl(curritem_); // Oder: expr(curritem_)
    while (last != curritem) {
	// Letzten Durchlauf durch diese Klammer kopieren.
	Pass pass = +item(passes_)[Z];
	item(passes_, 1|Z, pass);

	// Letzten Eintrag dieses Durchlaufs kopieren.
	last = pass(branch_)[Z];
	item = +last;
	pass(branch_, 1|Z, item);
    }

    // curritem der Kopie auf item setzen.
    // (currpart ist durch das anfängliche Kopieren bereits korrekt.)
    return dupl(curritem_, item);
}

// Den vorderen bzw. hinteren Operanden des Ausdrucks expr liefern,
// falls vorhanden, andernfalls nil.
Item id (Item item) {
    return item;
}
Expr front (Expr expr) {
    return traverseA(expr(row_), id)(opnd_);
}
Expr back (Expr expr) {
    return traverseZ(expr(row_), id)(opnd_);
}

// Import-, Export- oder Ausschlussangabe spec expandieren.
// expr ist der Ausdruck, zu dessen Operator die Angabe gehört.
// all ist die "Grundmenge".
Cat expand (Expr spec, Expr expr, Cat all) {
    // Es wird i. w. der Exportkatalog von spec geliefert,
    // aber dieser kann besonders interpretierte Operatoren enthalten:
    seq<Oper> opers;
    for (Oper oper : spec(expt_)(opers_)) {
	// All wird durch alle Operatoren des Katalogs all ersetzt.
	if (oper == All) {
	    opers += all(opers_);
	}
	// Self wird durch alle Operatoren des Exportkatalogs von expr
	// ersetzt.
	else if (oper == Self) {
	    opers += expr(expt_)(opers_);
	}
	// Ein Parameter des Ausdrucks expr wird durch alle Operatoren
	// des Exportkatalogs des korrespondierenden Operanden ersetzt.
	else if (Item item = search(oper, expr)) {
	    opers += item(opnd_)(expt_)(opers_);
	}
	// Jeder andere Operatoren steht für sich selbst.
	else {
	    opers += oper;
	}
    }
    return cat(opers);
}

// Archiv-Tabelle für den Parser.
// Die Tabelle ist global definiert, damit sowohl arch() als
// auch clear_tab() auf diese zugreifen können.
unordered_map<Key, Arch> tab;

// Löschen der gesammten Archiv-Tabelle.
// Dies ist nötig, um nach einem Parsevorgang die
// tabelle wieder zurück zu setzen (derzeit nur
// benötigt für den repl).
void clear_tab() {
	events.push_back(new message_event("Alle Archive werden gelöscht"));
	tab.clear();
}

// Archiv mit Position pos und Katalog cat liefern.
// Wenn es noch nicht existiert, wird es erzeugt und sein
// Ausschlusskatalog mit cat initialisiert (was für alle neuen
// Archive außer dem von parse erzeugten "Hauptarchiv" passend ist).
Arch arch(posA pos, Cat cat) {
	//static unordered_map<Key, Arch> tab;
	Arch& arch = tab[pair(pos, cat)];
	if(!arch) {
		arch = Arch(excl_, cat)(pos_, pos);
		events.push_back(new create_archive_event(pos - A));
	}
	return arch;
}

// Notwendig für unordered_set<Part>.
template <>
struct std::hash<Part> : CH::hash<Part> {};

// Hinweise an proceed:

// Der übergebene Ausdruck muss noch weiter fortgesetzt werden, d. h.
// er darf erst nach einem weiteren extend an finish übergeben werden.
const int Extend = 1 << 0;

// Der übergebene Ausdruck darf nicht weiter fortgesetzt werden,
// d. h. er muss sofort an Finish übergeben werden.
const int Finish = 1 << 1;

// currpart und curritem bezeichnen nicht das Signaturteil und den
// Eintrag, der zuletzt mit Information gefüllt wurde, sondern der
// als nächstes gefüllt werden muss.
const int Next = 1 << 2;

// Vorabdeklaration von proceed (entweder rekursiv oder iterativ),
// um die beiden Zyklen im "Herzen" des Parsers aufzubrechen.
namespace recursive {
void proceed (Expr expr, int flags = 0,
			shared_ptr<unordered_set<Part>> currparts =
				make_shared<unordered_set<Part>>());
}
namespace iterative {
void proceed (Expr expr, int flags = 0);
}
#ifdef FLXC_RECURSIVE
using namespace recursive;
#else
using namespace iterative;
#endif

// Initialen Ausdruck mit Operator oper, Anfangsposition pos und
// Importkatalog impt erzeugen und zur Weiterverarbeitung an proceed
// weiterleiten.
void initial (Oper oper, posA pos, Cat impt) {
    // TODO: Warum gibt pos statt pos - A nicht das Gewünschte aus?
    TRBL("initial", oper, pos - A)
    Expr cons = Expr(oper_, oper)(impt_, impt)(beg_, pos)(end_, pos);
    proceed(cons);
}

// Unvollständigen Ausdruck cons und vollständigen Ausdruck comp
// eines Archivs kombinieren.
void combine (Expr cons, Expr comp) {
    TRBL("combine", cons, comp)

    Par par = cons(currpart_)(par_);
    Cat impt = comp(impt_);
    bool front = cons(beg_) == cons(end_);
    bool3 back = cons(currpart_)(back_);

    // Ausschlussangaben des Parameters überprüfen.
    int flag = 0;
    for (Excl excl : par(excls_)) {
	// Gilt die Angabe eventuell für den aktuellen Operanden?
	bool f = excl(front_) && front;
	bool m = excl(middle_) && !front && back <= maybe;
	bool b = excl(back_) && back >= maybe;
	if (!(f || m || b)) continue;

	// Angabe expandieren.
	Cat cat = expand(excl(expr_), cons, impt);
	bool hit = false;

	// Wenn sich die Angabe auf den linken Rand bezieht:
	if (excl(left_)) {
	    for (Expr left = comp; left; left = ::front(left)) {
		if (left(oper_) << cat) {
		    TRLN("operator at left border excluded")
		    hit = true;
		}
	    }
	}

	// Wenn sich die Angabe auf die Spitze bezieht:
	if (excl(top_)) {
	    if (comp(oper_) << cat) {
		TRLN("operator at top excluded")
		hit = true;
	    }
	}

	// Wenn sich die Angabe auf den rechten Rand bezieht:
	if (excl(right_)) {
	    for (Expr right = comp; right; right = ::back(right)) {
		if (right(oper_) << cat) {
		    TRLN("operator at right border excluded")
		    hit = true;
		}
	    }
	}

	// Liegt ein Verstoß vor?
	if (!hit) continue;

	// Wenn die Angabe für den vorderen Operanden gilt
	// und der aktuelle Operand der vordere ist: Fehler.
	if (f) {
	    return;
	}

	// Wenn die Angabe für einen mittleren Operanden gilt
	// und der aktuelle Operand ein mittlerer sein könnte:
	if (m) {
	    // Wenn es garantiert ein mittlerer ist: Fehler.
	    // Andernfalls darf es letztlich kein mittlerer sein,
	    // d. h. der Ausdruck darf nicht weiter fortgesetzt werden.
	    // Wenn flag aber bereits Extend ist: Fehler.
	    if (back == false || flag == Extend) return;
	    flag = Finish;
	}

	// Wenn die Angabe für den letzten Operanden gilt
	// und der aktuelle Operand der letzte sein könnte:
	if (b) {
	    // Wenn es garantiert der letzte ist: Fehler.
	    // Andernfalls darf es letztlich nicht der rechte sein,
	    // d. h. der Ausdruck muss noch weiter fortgesetzt werden.
	    // Wenn flag aber bereits Finish ist: Fehler.
	    if (back == true || flag == Finish) return;
	    flag = Extend;
	}
    }

    // Der Typ von comp muss zum Typ von par passen.
    // Das ist momentan immer der Fall.

	// comp als aktuellen Operanden
	// zu einem Duplikat comb von cons hinzufügen.
	// Endposition von comb anpassen und an proceed weiterleiten.
	Expr comb = dupl(cons);
	comb(curritem_)(opnd_, comp);
	comb(end_, comp(end_));

	events.push_back(new event_group({
			new expr_gets_used_event(comp(beg_) - A, cons),
			new expr_gets_used_event(comp(beg_) - A, comp)}));
	events.push_back(new event_group({
			new expr_no_longer_gets_used_event(comp(beg_) - A, cons),
			new expr_no_longer_gets_used_event(comp(beg_) - A, comp)
	}));
	proceed(comb, flag);
}

// Vollständigen Ausdruck comp zum passenden Archiv hinzufügen und
// jeden unvollständigen Ausdruck des Archivs mit comp kombinieren.
void publish (Expr comp) {
    // Passendes Archiv a ermitteln.
    Arch a = arch(comp(beg_), comp(impt_));

    // TODO:
    // Wenn das Archiv bereits einen Ausdruck mit gleicher Endposition,
    // gleichem Typ und gleichem Exportkatalog enthält, soll comp nicht
    // hinzugefügt werden, sondern stattdessen eine Fehlermeldung über
    // Mehrdeutigkeit ausgegeben werden.

    // comp zum Archiv hinzufügen.
	events.push_back(new add_comp_event(a(pos_) - A, comp));
    a(comp_, Z, comp);

    // Jeden unvollständigen Ausdruck cons des Archivs
    // mit comp kombinieren.
    // Beim rekursiven Algorithmus besteht die Möglichkeit, dass die
    // Sequenz a(cons_) während ihrer Abarbeitung hier von rekursiv
    // aufgerufenen Funktionen erweitert wird.
    // Da hier aber eine Kopie der aktuellen Sequenz durchlaufen wird,
    // stellt dies kein Problem dar.
    // Und tatsächlich dürfen nur die momentan vorhandenen Elemente
    // durchlaufen werden, weil neu hinzugefügte Elemente bereits
    // anderweitig verarbeitet werden.
    for (Expr cons : a(cons_)) combine(cons, comp);
}

// Unvollständigen Ausdruck cons zum passenden Archiv hinzufügen und
// mit jedem vollständigen Ausdruck des Archivs kombinieren.
void subscribe (Expr cons) {
    TRBL("subscribe", cons)

    // Importkatalog des aktuellen Operanden von cons und das zugehörge
    // Archiv ermitteln.
    Par par = cons(currpart_)(par_);
    posA pos = cons(end_);
    Cat impt = par(impt_) ? expand(par(impt_), cons, cons(impt_))
							: cons(impt_);
    Arch a = arch(pos, impt);

    // Katalog der garantiert ausgeschlossenen Operatoren des aktuellen
    // Operanden von cons ermitteln.
    bool front = cons(beg_) == cons(end_);
    bool3 back = cons(currpart_)(back_);
    Cat cat;
    for (Excl excl : par(excls_)) {
	// Gilt die Ausschlussangabe excl garantiert für den aktuellen
	// Operanden?
	if (excl(front_) && front
	 || excl(middle_) && !front && back == false
	 || excl(back_) && back == true) {
	    cat += expand(excl(expr_), cons, impt);
	}
    }

    // Ausschlusskatalog des Archivs aktualisieren und nebenbei die
    // Menge inits derjenigen Operatoren ermitteln, für die initiale
    // Ausdrücke erzeugt werden müssen.
    // a(excl_) = a(excl_) & cat, inits = a(excl_) - cat.
    TRLN("intersect", a(excl_), cat)
    seq<Oper> inits;
    a(excl_, intersect(a(excl_), cat, inits));
    TRLN("result", a(excl_), inits)

    // cons zum Archiv hinzufügen
    // und mit den vollständigen Ausdrücken des Archivs kombinieren.
    // Vgl. auch Anmerkung zur Iteration durch a(cons_) in publish.
	events.push_back(new add_cons_event(a(pos_) - A, cons));
    a(cons_, Z, cons);
    for (Expr comp : a(comp_)) combine(cons, comp);

    // Ggf. initiale Ausdrücke erzeugen.
    for (Oper oper : inits) initial(oper, pos, impt);
}

// Vollständigen Ausdruck comp fertigstellen
// und zum passenden Archiv hinzufügen.
void finish (Expr comp) {
    TRBL("finish", comp)

    // Später: Typ des Ausdrucks ermitteln.

    // Ggf. Exportkatalog ermitteln.
    if (!comp(expt_)) {
	comp(expt_, expand(comp(oper_)(expt_), comp, comp(impt_)));
    }

    // Später: Wenn der Ausdruck eine Anwendung eines virtuellen
    // Operators ist, durch die entsprechende Realisierung ersetzen.

    // Ausdruck zum passenden Archiv hinzufügen.
    publish(comp);
}

// Unvollständigen Ausdruck cons erweitern,
// d. h. den aktuellen Eintrag mit Information füllen.
// Resultatwert true genau dann, wenn die aufrufende Funktion proceed
// die Verarbeitung von cons fortsetzen soll.
bool extend (Expr cons) {
    TRBL("extend", cons)

    // Als erstes eventuellen Zwischenraum überlesen.
    posA pos = cons(end_);
    scan_white(pos);
    cons(end_, pos);

    // Wenn das aktuelle Signaturteil ein Parameter ist,
    // Ausdruck zum passenden Archiv hinzufügen.
    Part part = cons(currpart_);
    if (part(par_)) {
	subscribe(cons);
	return false;
    }

    // Andernfalls ist das aktuelle Signaturteil ein Name.
    // Versuchen, einen zum Namen name passenden Text word zu lesen.
    str name = part(name_);
    str word;
    if (part(reg_)) {
	if (!scan_match(pos, name, word)) return false;
    }
    else {
	if (!scan_exact(pos, name)) return false;
	word = name;
    }

	// Gelesenen Text speichern, Endposition des Ausdrucks weitersetzen
	// und Ausdruck an proceed zurückgeben.

	cons(curritem_)(word_, word);
	cons(end_, pos);
	TRLN("after successfull scan", cons)
	return true;
}

// Prolog- oder Epilogfunktion des aktuellen Signaturteils des Ausdrucks
// expr ausführen.
template <typename A>
void exec (Expr expr, A a_) {
    if (auto f = expr(currpart_)(a_)) f(expr);
}

namespace recursive {

// Alle möglichen Fortsetzungen des Ausdrucks cons konstruieren und an
// die passenden Funktionen bzw. Archive weiterleiten.
// currpart und curritem bezeichnen normalerweise das Signaturteil und
// den Eintrag des Ausdrucks, der zuletzt mit Information gefüllt wurde
// (oder sie sind nil), es sei denn flags enthält Next.
// currparts ist bei einem Aufruf von außen leer und wird an rekursive
// Aufrufe weitergegeben.
void proceed (Expr cons, int flags,
			shared_ptr<unordered_set<Part>> currparts) {
    TRBL("proceed")
    while (true) {
	TRBL("while", cons, flags)

	// Hilfsfunktion:
	// Wenn das aktuelle Signaturteil eine Klammer ist, für jede
	// Alternative der Klammer einen (weiteren) Durchlauf beginnen,
	// sofern dies nicht wegen Finish unnötig ist.
	auto paren = [&] () {
	    if (flags&Finish) return;
	    posA a;
	    for (Sig alt : cons(currpart_)(alts_)) {
		Expr copy = dupl(cons);
		down(copy, ++a);
		proceed(copy, flags|Next, currparts);
	    }
	};

	// Fehlermeldung ausgeben und abbrechen.
	auto error = [&] () {
	    cerr << "Operator of expression starting at "
		<< "input position " << cons(beg_) / A
		<< " should be rewritten," << endl
		<< "because its use might produce "
		<< "inherently ambiguous expressions."
		<< endl;
	    exit(1);
	};

	// Überprüfen, ob innerhalb der aktuellen "Hauptausführung" von
	// proceed inklusive rekursiver Aufrufe für Alternativen von
	// Klammern dasselbe Signaturteil part schon einmal erreicht
	// wurde.
	// Wenn ja, Abbruch, weil der Operator von cons dann "inhärent
	// mehrdeutig" ist, was bei Wiederholungsklammern zu Endlos-
	// ausführungen führen würde und grundsätzlich nicht sinnvoll
	// ist.
	// part ist entweder cons(currpart_) oder nil, was logisch
	// dem Ende der Gesamtsignatur entspricht.
	auto check = [&] (Part part) {
	    if (currparts->count(part)) error();
	    currparts->insert(part);
	};

	// Sofern flags nicht Next enthält:
	// Epilogfunktion des aktuellen Signaturteils ausführen und,
	// wenn möglich, in der Signatur einen Schritt nach rechts
	// gehen. Wenn bzw. solange dies nicht möglich ist:
	while (!(flags&Next) && (exec(cons, after_), !right(cons))) {
	    TRLN("while")

	    // Wenn möglich, eine Ebene nach oben gehen.
	    // Wenn dies nicht möglich ist, ist der Ausdruck fertig
	    // und wird an finish übergeben, sofern dies nicht durch
	    // Extend verboten wird.
	    if (!up(cons)) {
		TRLN("if")
		if (!(flags&Extend)) {
		    check(nil);
		    finish(cons);
		}
		return;
	    }

	    // Die folgende Überprüfung ist wegen check nicht mehr
	    // nötig.
	    #if 0
	    // Hier befindet sich eine Klammer in der Signatur.
	    // Wenn ihr letzter Durchlauf "faktisch leer" ist,
	    // d. h. keinen atomaren Eintrag enthält:
	    if (!traverseA(cons(curritem_)(passes_)[Z](branch_),
					[] (Item) { return true; })) {
		// Wenn die Klammer optional und/oder wiederholbar ist,
		// stellt das eine inhärente Mehrdeutigkeit dar,
		// die bei einer wiederholbaren Klammer zu einem
		// Endlosslauf des Parsers führen würde.
		if (cons(currpart_)(opt_) || cons(currpart_)(rep_)) {
		    error();
		}
	    }
	    #endif

	    // Wenn sich dort eine Wiederholungsklammer befindet:
	    // Einen weiteren Durchlauf durch die Klammer beginnen
	    // (sofern dies nicht wegen Finish unnötig ist).
	    if (cons(currpart_)(rep_)) paren();
	}
	TRLN("after while")

	check(cons(currpart_));

	// Für eventuelle Fortsetzungen der Schleife Next aus flags
	// entfernen (aber eventuell vorhandene andere Werte lassen!).
	flags &= ~Next;

	// Prologfunktion des neuen aktuellen Signaturteils ausführen.
	exec(cons, before_);

	// Wenn das neue aktuelle Signaturteil atomar ist,
	// Ausdruck an extend übergeben.
	// (Wenn flags Finish enthält, sollte dieser Fall nicht
	// auftreten.)
	if (!cons(currpart_)(alts_)) {
	    // Wenn extend das nächste Wort hinzufügen konnte
	    // (Resultatwert true), weitermachen.
	    // Die Bedingung Extend ist dann bereits erfüllt.
	    TRLN("atomic part")
	    if (extend(cons)) {
		// Damit beim Weitermachen eine neue leere Menge
		// currparts verwendet wird, kann die Schleife nicht
		// einfach fortgesetzt werden; deshalb ein rekursiver
		// Aufruf von proceed.
		///flags &= ~Extend;
		///continue;
		proceed(cons);
	    }
	    // Andernfalls abbrechen, weil extend entweder das nächste
	    // Wort nicht hinzufügen konnte oder den Ausdruck an
	    // subscribe übergeben hat.
	    return;
	}

	// Andernfalls ist dieses Teil eine Klammer.
	paren();

	// Bei eckigen und geschweiften Klammern von vorne beginnen,
	// d. h. die Klammer überspringen, sonst Ende.
	if (!cons(currpart_)(opt_)) return;
    }
}

void process () {}

}

namespace iterative {

// Wenn der Algorithmus iterativ arbeitet, speichert proceed seine
// Parameter in einer Warteschlange, die von process abgearbeitet wird.
// queue ist für diesen Zweck einfacher und effizienter als seq.
queue<pair<Expr, int>> q;

void proceed (Expr cons, int flags) {
	//std::cout << "proceed:\tAusdruck '" << get_scanned_str_for_expr(cons) << "' in die Queue abgelegt" << std::endl;
    q.push(make_pair(cons, flags));
}

void process () {
    while (!q.empty()) {
	auto pair = q.front();
	//std::cout << "process:\tAusdruck '" << get_scanned_str_for_expr(pair.first) << "' aus der Queue herausgenommen" << std::endl;
	q.pop();
	recursive::proceed(pair.first, pair.second);
    }
}

}

// Alle vollständigen Ausdrücke liefern, die aus den Operatoren opers
// aufgebaut sind und vom Anfang bis zum Ende der Eingabe gehen.
seq<Expr> parse (const seq<Oper>& opers) {
    // Anfänglichen Zwischenraum überlesen.
    posA pos = A;
    scan_white(pos);

    // "Hauptarchiv" a mit leerem Ausschlusskatalog erzeugen.
    Cat impt = cat(opers);
    Arch a = arch(pos, impt)(excl_, cat());

    // Attribut back in den Signaturen aller Operatoren setzen.
    for (Oper oper : opers) setback(oper(sig_));

    // Initiale Ausdrücke mit Anfangsposition pos und Importkatalog
    // opers erzeugen.
    // Beim rekursiven Algorithmus wird dadurch der gesamte
    // Parse-Vorgang durchgeführt.
    for (Oper oper : opers) initial(oper, pos, impt);

    // Beim iterativen Algorithmus muss anschließend die Warteschlange
    // von proceed abgearbeitet werden.
    process();

    // Alle vollständigen Ausdrücke des Hauptarchivs ermitteln,
    // die bis zum Ende der Eingabe gehen.
    seq<Expr> exprs;
    for (Expr expr : a(comp_)) {
	posA pos = expr(end_);
	scan_white(pos);
	if (scan_eof(pos)) exprs += expr;
    }
    return exprs;
}

// Ausdruck expr baumartig mit anfänglicher Einrückung ind ausgeben.
void print (Expr expr, str ind) {
    traverseA(expr(row_), [&] (Item item) {
	if (str word = item(word_)) {
	    cout << ind << word << endl;
	}
	else {
	    print(item(opnd_), ind + "  ");
	}
	return false;
    });
}