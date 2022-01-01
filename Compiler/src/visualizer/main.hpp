#pragma once

#include "constants.hpp"

#include <cassert>
#include <ncurses.h>
#include <vector>

#include "archive.hpp"
#include "event.hpp"
#include "utils.hpp"

/*
operator catalog sichtbar machen (katalog nummer) Katalog der in Key drin steckt
katalog übersicht im unteren bereich (matrix darstellung, zeilenweise untereinander sämtliche operatoren die in irgend einem Katalog auftauchen, nach rechts rüber katalog 1 katalog 2 katalog drei)

vll auch ausgeschlossene operatoren (excl_)
*/

using event_iterator = std::vector<event*>::iterator;
extern event_iterator current_event_it;

void handleSignal(int sig);

void setup_colors();

bool load_state(const event_iterator& target_event_it);

int start_vistalizer(const CH::str& source_string);