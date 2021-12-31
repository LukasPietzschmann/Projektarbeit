#pragma once

#define FOOTER_HEIGHT 1
#define HEADER_HEIGHT 1
#define MSG_BUS_WIDTH 35

#define COLOR_GREY 50
#define COLOR_DARK_GREY 51

#define STD_COLOR_PAIR 1
#define HEADER_COLOR_PAIR 99
#define FOOTER_COLOR_PAIR 98
#define MSG_BUS_COLOR_PAIR 97

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

int start_vistalizer(const std::string& source_string);