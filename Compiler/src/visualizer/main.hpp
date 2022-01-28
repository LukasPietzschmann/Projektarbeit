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

void handleSignal(int sig);

void setup_colors();

bool step_forward();
bool step_backward();

int start_visualizer(const CH::str& source_string);