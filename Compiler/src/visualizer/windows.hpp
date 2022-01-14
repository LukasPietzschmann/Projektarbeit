#pragma once

#include "archive.hpp"
#include <ncurses.h>
#include <vector>

extern std::vector<archive> arch_windows;

extern WINDOW* src_display;
extern WINDOW* footer;
extern WINDOW* queue_display_pad;
extern WINDOW* main_viewport;

extern int viewport_scroll_y;

#define refresh_main_viewport(void) pnoutrefresh(main_viewport, viewport_scroll_y, 0, HEADER_HEIGHT, 0, height - (HEADER_HEIGHT + FOOTER_HEIGHT), width - QUEUE_WIDTH)