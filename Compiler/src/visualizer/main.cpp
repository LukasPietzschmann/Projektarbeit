#include "main.hpp"

/*
operator catalog sichtbar machen (katalog nummer) Katalog der in Key drin steckt
katalog übersicht im unteren bereich (matrix darstellung, zeilenweise untereinander sämtliche operatoren die in irgend einem Katalog auftauchen, nach rechts rüber katalog 1 katalog 2 katalog drei)

vll auch ausgeschlossene operatoren (excl_)
*/

WINDOW* footer;
WINDOW* src_display;
WINDOW* msg_bus;

std::vector<archive> arch_windows;
std::vector<event*> events;
event_iterator current_event_it;

void handleSignal(int sig) {
	endwin();
	exit(sig);
}

void setup_colors() {
	start_color();
	init_color(COLOR_GREY, 179, 179, 179);
	init_color(COLOR_DARK_GREY, 74, 74, 74);
	init_pair(STD_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(FOOTER_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
	init_pair(HEADER_COLOR_PAIR, COLOR_WHITE, COLOR_GREY);
	init_pair(MSG_BUS_COLOR_PAIR, COLOR_WHITE, COLOR_DARK_GREY);
}

bool load_state(const event_iterator& target_event_it) {
	if(target_event_it == events.end() || target_event_it == events.begin() - 2)
		return false;

	const long distance = std::distance(current_event_it, target_event_it);

	if(distance == 0)
		return true;

	if(distance > 0) {// vorwärts
		for(auto it = current_event_it + 1; it != target_event_it + 1; ++it)
			(*it)->exec();
	}else if(distance < 0) {// rückwärts
		for(auto it = current_event_it; it != target_event_it; --it)
			(*it)->undo();
	}

	current_event_it = target_event_it;

	return true;
}

int start_vistalizer(const std::string& source_string) {
	int width, height;

	initscr();

	signal(SIGINT, handleSignal);
	signal(SIGABRT, handleSignal);
	signal(SIGKILL, handleSignal);

	timeout(-1);
	noecho();
	setup_colors();
	curs_set(0);
	getmaxyx(stdscr, height, width);
	wresize(stdscr, height - (FOOTER_HEIGHT + HEADER_HEIGHT), width - MSG_BUS_WIDTH);
	mvwin(stdscr, 1, 0);

	bkgd(COLOR_PAIR(STD_COLOR_PAIR));

	footer = newwin(FOOTER_HEIGHT, width - MSG_BUS_WIDTH, height - FOOTER_HEIGHT, 0);
	wbkgd(footer, COLOR_PAIR(FOOTER_COLOR_PAIR));
	center_text_hor(footer, "q: quit    n: next    p: previous", 0);

	src_display = newwin(HEADER_HEIGHT, width - MSG_BUS_WIDTH, 0, 0);
	wbkgd(src_display, COLOR_PAIR(HEADER_COLOR_PAIR));
	mvwaddstr(src_display, 0, 0, source_string.c_str());

	msg_bus = newwin(height, MSG_BUS_WIDTH, 0, width - MSG_BUS_WIDTH);
	wbkgd(msg_bus, COLOR_PAIR(MSG_BUS_COLOR_PAIR));
	scrollok(msg_bus, true);

	refresh();
	wrefresh(footer);
	wrefresh(src_display);
	wrefresh(msg_bus);

	/*events.push_back(new create_archive_event(10));
	events.push_back(new add_comp_event(10, "test"));
	events.push_back(new add_cons_event(10, "test2"));
	events.push_back(new add_comp_event(10, "test3"));
	events.push_back(new add_comp_event(10, "test4"));
	events.push_back(new add_cons_event(10, "test5"));
	events.push_back(new create_archive_event(2));
	events.push_back(new add_comp_event(2, "test3"));*/

	current_event_it = events.begin() - 1;
	load_state(current_event_it);

	while(char c = wgetch(footer)) {
		if(c == 'q')
			break;

		bool worked = false;
		if(c == 'n')
			worked = load_state(current_event_it + 1);
		else if(c == 'p')
			worked = load_state(current_event_it - 1);

		if(!worked) {
			beep();
			flash();
		}
	}

	endwin();
	return 0;
}