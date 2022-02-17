#pragma once

#include <cassert>
#include <csignal>
#include <map>
#include <ncurses.h>
#include <panel.h>
#include <vector>

#include "archive.hpp"
#include "constants.hpp"
#include "event.hpp"
#include "expr_queue.hpp"
#include "popup.hpp"
#include "popup_manager.hpp"
#include "scrollable.hpp"
#include "utils.hpp"

/*
Davids Code übernehmen

Vielleicht seitwärts scrollen

Noch ein Paar Kommentare
*/

using event_iterator = std::vector<event*>::iterator;

enum input_state {
	s_wait_for_marker = 1 << 0,
	s_create_marker = 1 << 1,
	s_read_marker = 1 << 2,
	s_any_input = 1 << 3
};

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
void setup_windows();

/**
 * @return `true`, falls noch ein nächstes Event existiert hat,
 * sonst `false`
 */
bool step_n_events_forward(int n);

/**
 * @return `true`, falls noch ein vorheriges Event existiert hat,
 * sonst `false`
 */
bool step_n_events_backward(int n);

int start_visualizer(const CH::str& source_string, int event_to_skip_to);