#include "main.hpp"

/*
operator catalog sichtbar machen (katalog nummer) Katalog der in Key drin steckt
katalog übersicht im unteren bereich (matrix darstellung, zeilenweise untereinander sämtliche operatoren die in irgend einem Katalog auftauchen, nach rechts rüber katalog 1 katalog 2 katalog drei)

vll auch ausgeschlossene operatoren (excl_)
*/

WINDOW* footer;
WINDOW* src_display;
WINDOW* queue_display_pad;

int screen_center;
int src_str_center;
int src_str_len;
int width;
int height;

std::vector<archive> arch_windows;
std::vector<event*> events;
event_iterator next_event_it;

void handleSignal(int sig) {
	arch_windows.clear();
	events.clear();
	delwin(footer);
	delwin(src_display);
	delwin(queue_display_pad);
	endwin();
	exit(sig);
}

void setup_colors() {
	start_color();
	init_color(COLOR_LIGHT_GREY, 224, 224, 224);
	init_color(COLOR_GREY, 179, 179, 179);
	init_color(COLOR_DARK_GREY, 74, 74, 74);
	init_pair(STD_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(FOOTER_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
	init_pair(HEADER_COLOR_PAIR, COLOR_WHITE, COLOR_GREY);
	init_pair(QUEUE_COLOR_PAIR, COLOR_WHITE, COLOR_DARK_GREY);
	init_pair(HIGHLIGHT_EXPR_COLOR_PAR, COLOR_GREEN, COLOR_BLACK);
	init_pair(MUTED_COLOR_PAIR, COLOR_LIGHT_GREY, COLOR_BLACK);
}

bool step_forward() {
	if(next_event_it == events.end())
		return false;

	if((*(next_event_it++))->exec() == event::did_nothing)
		step_forward();

	return true;
}

bool step_backward() {
	if(next_event_it == events.begin())
		return false;

	if((*(--next_event_it))->undo() == event::did_nothing)
		step_backward();

	return true;
}

int start_visualizer(const CH::str& source_string) {
	src_str_len = *source_string;
	src_str_center = src_str_len / 2;

	initscr();

	signal(SIGINT, handleSignal);
	signal(SIGABRT, handleSignal);
	signal(SIGKILL, handleSignal);

	timeout(-1);
	noecho();
	setup_colors();
	curs_set(0);
	getmaxyx(stdscr, height, width);
	screen_center = width / 2;
	wresize(stdscr, height - (FOOTER_HEIGHT + HEADER_HEIGHT), width - QUEUE_WIDTH);
	mvwin(stdscr, HEADER_HEIGHT, 0);

	bkgd(COLOR_PAIR(STD_COLOR_PAIR));

	footer = newwin(FOOTER_HEIGHT, width - QUEUE_WIDTH, height - FOOTER_HEIGHT, 0);
	wbkgd(footer, COLOR_PAIR(FOOTER_COLOR_PAIR));
	center_text_hor(footer, "q: quit    n: next    p: previous", 0);

	src_display = newwin(HEADER_HEIGHT, width - QUEUE_WIDTH, 0, 0);
	wbkgd(src_display, COLOR_PAIR(HEADER_COLOR_PAIR));
	mvwaddstr(src_display, 0, width / 2 - *source_string / 2, &source_string.elems[0]);

	queue_display_pad = newpad(height * 2, width * 2);
	wbkgd(queue_display_pad, COLOR_PAIR(QUEUE_COLOR_PAIR));

	pnoutrefresh(queue_display_pad, 0, 0, 0, width - QUEUE_WIDTH, height - 1, width - 1);
	wnoutrefresh(stdscr);
	wnoutrefresh(src_display);
	wnoutrefresh(footer);
	doupdate();

	next_event_it = events.begin();

	while(char c = getch()) {
		if(c == 'q')
			break;

		bool worked = false;
		if(c == 'n')
			worked = step_forward();
		else if(c == 'p')
			worked = step_backward();
		else if(c == 'w') {
			expr_queue::the().scroll_y(-1);
			worked = true;
		}else if(c == 's') {
			expr_queue::the().scroll_y(1);
			worked = true;
		}

		if(!worked) {
			beep();
			flash();
		}
	}

	handleSignal(0);
	return 0;
}