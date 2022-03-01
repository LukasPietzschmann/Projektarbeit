#pragma once

#include <cassert>
#include <csignal>
#include <map>
#include <ncurses.h>
#include PANEL_HEADER
#include <vector>

#include "archive.hpp"
#include "constants.hpp"
#include "event.hpp"
#include "expr_queue.hpp"
#include "popup.hpp"
#include "popup_manager.hpp"
#include "scrollable.hpp"
#include "utils.hpp"

using event_iterator = std::vector<event*>::iterator;

enum input_state {
	s_wait_for_marker = 1 << 0,
	s_create_marker = 1 << 1,
	s_read_marker = 1 << 2,
	s_any_input = 1 << 3
};

int start_visualizer(const CH::str& source_string, int event_to_skip_to = 0);