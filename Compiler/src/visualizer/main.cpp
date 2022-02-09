#include "main.hpp"

WINDOW* footer;
WINDOW* src_display;
scrollable* queue_display;
scrollable* main_viewport;
popup* opers_popup;

int main_viewport_center;
int src_str_center;
int src_str_len;
int width;
int height;

std::vector<archive> arch_windows;
std::vector<event*> events;
event_iterator next_event_it;
uint64_t current_event_index;

void handleSignal(int sig) {
	arch_windows.clear();
	events.clear();
	delwin(footer);
	delwin(src_display);
	delete queue_display;
	delete main_viewport;
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
	init_pair(AMBIGUOUS_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
}

bool step_one_event_forward() {
	if(next_event_it == events.end())
		return false;

	++current_event_index;
	// Hatte das aktuelle event keine Auswirkung, wird direkt das Nächste ausgeführt
	if((*(next_event_it++))->exec() == event::did_nothing)
		step_one_event_forward();

	return true;
}

bool step_one_event_backward() {
	if(next_event_it == events.begin())
		return false;

	--current_event_index;
	// Hatte das aktuelle event keine Auswirkung, wird direkt das Nächste ausgeführt
	if((*(--next_event_it))->undo() == event::did_nothing)
		step_one_event_backward();

	return true;
}

int start_visualizer(const CH::str& source_string, int event_to_scip_to) {
	assert(event_to_scip_to >= 0);

	src_str_len = *source_string;
	src_str_center = src_str_len / 2;

	initscr();

	signal(SIGINT, handleSignal);
	signal(SIGABRT, handleSignal);
	signal(SIGKILL, handleSignal);

	cbreak();
	timeout(-1);
	noecho();
	setup_colors();
	curs_set(0);
	getmaxyx(stdscr, height, width);
	//Wird der stdsrc nicht minimal klein gemacht, verdeckt er den Rest
	wresize(stdscr, 0, 0);
	wnoutrefresh(stdscr);

	main_viewport = new scrollable(width - QUEUE_WIDTH - 1, height - HEADER_HEIGHT - FOOTER_HEIGHT - 1, 0,
			HEADER_HEIGHT);
	wbkgd(**main_viewport, COLOR_PAIR(STD_COLOR_PAIR));
	main_viewport->prepare_refresh();

	main_viewport_center = main_viewport->get_width() / 2;

	footer = newwin(FOOTER_HEIGHT, width - QUEUE_WIDTH, height - FOOTER_HEIGHT, 0);
	wbkgd(footer, COLOR_PAIR(FOOTER_COLOR_PAIR));
	center_text_hor(footer, "q: quit    n: next    p: previous    o: toggle opers", 0);

	src_display = newwin(HEADER_HEIGHT, width - QUEUE_WIDTH, 0, 0);
	wbkgd(src_display, COLOR_PAIR(HEADER_COLOR_PAIR));
	mvwaddstr(src_display, 0, getmaxx(src_display) / 2 - *source_string / 2, &source_string.elems[0]);

	queue_display = new scrollable(QUEUE_WIDTH - 1, height - 1, width - QUEUE_WIDTH, 0);
	wbkgd(**queue_display, COLOR_PAIR(QUEUE_COLOR_PAIR));
	queue_display->prepare_refresh();

	wnoutrefresh(src_display);
	wnoutrefresh(footer);

	auto* popup_win = new scrollable(POPUP_WIDTH, POPUP_HEIGHT, main_viewport_center - POPUP_WIDTH / 2, 5);
	wbkgd(**popup_win, COLOR_PAIR(POPUP_COLOR_PAIR));
	opers_popup = new popup(popup_win);

	next_event_it = events.begin();
	current_event_index = 0;

	for(int i = 0; i < event_to_scip_to; ++i)
		step_one_event_forward();

	doupdate();

	int multiplier = 0;
	while(char c = getch()) {
		if(c == 'q')
			break;

		bool worked = true;

		if(c >= '0' && c <= '9') {
			multiplier *= 10;
			multiplier += c - 48;
			continue;
		}

		multiplier = std::max(1, multiplier);

		if(c == 'o' && !opers_popup->toggle())
			main_viewport->prepare_refresh();
		switch(c) {
			case 'n':
				for(int i = 0; i < multiplier; ++i)
					worked = step_one_event_forward();
				break;
			case 'p':
				for(int i = 0; i < multiplier; ++i)
					worked = step_one_event_backward();
				break;
			case 66: // arrow down
				if(opers_popup->is_currently_shown())
					(**opers_popup)->scroll_y(multiplier * 1);
				else
					main_viewport->scroll_y(multiplier * 1);
				break;
			case 65: // arrow up
				if(opers_popup->is_currently_shown())
					(**opers_popup)->scroll_y(multiplier * -1);
				else
					main_viewport->scroll_y(multiplier * -1);
				break;
			case 's': queue_display->scroll_y(multiplier * 1);
				break;
			case 'w': queue_display->scroll_y(multiplier * -1);
				break;
		}

		wmove(src_display, 0, 0);
		wprintw(src_display, "Event: %3d", current_event_index);
		wnoutrefresh(src_display);

		// das muss als letztes vor `doupdate` ausgeführt werden, damit das popup,
		// falls es denn angezeigt wird, nicht vom rest überlagert wird
		if(opers_popup->is_currently_shown())
			opers_popup->prepare_refresh();

		//`doupdate` hier nötig, da alle aufgerufenen Funktionen in der Regel nu `wnoutrefresh` verwenden
		doupdate();

		if(!worked) {
			beep();
			flash();
		}

		multiplier = 0;
	}

	handleSignal(0);
	return 0;
}