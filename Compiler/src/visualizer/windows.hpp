#pragma once

#include "archive.hpp"
#include "scrollable.hpp"
#include "popup.hpp"
#include <ncurses.h>
#include <vector>

extern std::vector<archive> arch_windows;

extern WINDOW* src_display;
extern WINDOW* footer;
extern scrollable* queue_display;
extern scrollable* main_viewport;
extern popup* opers_popup;