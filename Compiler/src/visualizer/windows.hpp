#pragma once

#include "archive.hpp"
#include "scrollable.hpp"
#include <ncurses.h>
#include <vector>

extern std::vector<archive> arch_windows;

extern WINDOW* src_display;
extern WINDOW* footer;
extern WINDOW* queue_display_pad;
extern scrollable* main_viewport;