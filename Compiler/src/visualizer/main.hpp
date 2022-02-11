#pragma once

#include <cassert>
#include <ncurses.h>
#include <map>
#include <vector>
#include <csignal>
#include <panel.h>

#include "archive.hpp"
#include "constants.hpp"
#include "event.hpp"
#include "expr_queue.hpp"
#include "popup.hpp"
#include "utils.hpp"
#include "scrollable.hpp"

using event_iterator = std::vector<event*>::iterator;
extern event_iterator next_event_it;
extern uint64_t current_event_index;
extern CH::str src_str;

extern scrollable* current_scrollable;

extern popup_manager* p_manager;

enum input_state {
	s_wait_for_marker = 1 << 0,
	s_create_marker = 1 << 1,
	s_read_marker = 2 << 1,
	s_wait_for_scrollable_selection = 3 << 1,
	s_any_input = 4 << 1
};

extern int current_state;

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