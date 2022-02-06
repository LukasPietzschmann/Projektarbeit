#pragma once

#include <cassert>
#include <ncurses.h>
#include <vector>
#include <csignal>

#include "archive.hpp"
#include "constants.hpp"
#include "event.hpp"
#include "expr_queue.hpp"
#include "scrollable.hpp"
#include "utils.hpp"

/*
operator catalog sichtbar machen (katalog nummer) Katalog der in Key drin steckt
katalog übersicht im unteren bereich (matrix darstellung, zeilenweise untereinander sämtliche operatoren die in irgend einem Katalog auftauchen, nach rechts rüber katalog 1 katalog 2 katalog drei)

vll auch ausgeschlossene operatoren (excl_)
*/

using event_iterator = std::vector<event*>::iterator;
extern event_iterator next_event_it;

/**
 * Wird das Programm unerwartet durch ein Signal beendet,
 * MUSS der curses-Modus korrekt verlassen werden.
 * Dazu wird diese Funktion als Signal-Handler verwendet.
 */
void handleSignal(int sig);

/**
 * Hilfsfunktion um alle verwendeten Farben und Farb-Paare
 * zu initialisieren
 */
void setup_colors();

/**
 * @return `true`, falls noch ein nächstes Event existiert hat,
 * sonst `false`
 */
bool step_one_event_forward();
/**
 * @return `true`, falls noch ein vorheriges Event existiert hat,
 * sonst `false`
 */
bool step_one_event_backward();

int start_visualizer(const CH::str& source_string);