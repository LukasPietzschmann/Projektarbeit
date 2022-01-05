#include "logger.hpp"

static uint32_t next_row = 0;

void log(const CH::str& event) {
	if(*event > MSG_BUS_WIDTH) {
		std::string str(&event.elems[0]);
		str = str.substr(0, str.size() - ((*event - MSG_BUS_WIDTH) + 4));
		str += "...";
		mvwaddnstr(msg_bus, next_row++, 0, str.c_str(), str.size());
	}else
		mvwaddnstr(msg_bus, next_row++, 0, &event.elems[0], *event);
	wnoutrefresh(msg_bus);
}

void unlog() {
	if(next_row < 0)
		return;

	wmove(msg_bus, --next_row, 0);
	wclrtoeol(msg_bus);
	wnoutrefresh(msg_bus);
}