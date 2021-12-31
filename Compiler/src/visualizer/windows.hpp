#pragma once

#include "archive.hpp"
#include <ncurses.h>
#include <vector>

extern std::vector<archive> arch_windows;

extern WINDOW* src_display;
extern WINDOW* msg_bus;
extern WINDOW* footer;