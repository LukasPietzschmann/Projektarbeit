#pragma once

#include <cassert>
#include <ncurses.h>
#include <string>

#include "archive.hpp"

void center_text_hor(WINDOW* window, const std::string& str, uint32_t y_coordinate);