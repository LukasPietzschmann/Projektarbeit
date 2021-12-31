#include "logger.hpp"

static uint32_t next_row = 0;

void log(const std::string& event) {
	mvwaddnstr(msg_bus, next_row++, 0, event.c_str(), event.size());
	wrefresh(msg_bus);
}

void unlog() {
	if(next_row < 0)
		return;

	wmove(msg_bus, --next_row, 0);
	wclrtoeol(msg_bus);
	wrefresh(msg_bus);
}