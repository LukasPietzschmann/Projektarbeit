#pragma once

#include <ncurses.h>
#include <vector>

#include "archive.hpp"

class scrollable;
class popup;

extern std::vector<archive> arch_windows;

extern WINDOW* src_display;
extern WINDOW* footer;
extern scrollable* queue_display;
extern scrollable* main_viewport;
extern popup* opers_popup;
extern popup* help_popup;