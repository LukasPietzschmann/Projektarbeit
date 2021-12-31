#pragma once

#include <cassert>
#include <ncurses.h>

#include "archive.hpp"

void center_text_hor(WINDOW* window, const CH::str& str, uint32_t y_coordinate);