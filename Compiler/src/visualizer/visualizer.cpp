#include "visualizer.hpp"

WINDOW* footer;
WINDOW* src_display;
scrollable* queue_display;
scrollable* main_viewport;
popup* opers_popup;
popup* help_popup;

scrollable* current_scrollable;
scrollable* prev_scrollable;

int main_viewport_horizontal_center;
int main_viewport_vertical_center;
int src_str_center;
int width;
int height;
CH::str src_str;

std::vector<archive> arch_windows;
std::vector<event*> events;
event_iterator next_event_it;
uint64_t current_event_index;

std::map<char, int> markers;

int current_state;

/**
 * Wird das Programm unerwartet durch ein Signal beendet,
 * MUSS der curses-Modus korrekt verlassen werden.
 * Dazu wird diese Funktion als Signal-Handler verwendet.
 */
void handleSignal(int sig) {
	arch_windows.clear();
	for(const auto e: events)
		delete e;
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
	init_color(COLOR_LIGHT_GREY, 600, 600, 600);
	init_color(COLOR_GREY, 400, 400, 400);
	init_color(COLOR_DARK_GREY, 100, 100, 100);
	init_pair(STD_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(FOOTER_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
	init_pair(HEADER_COLOR_PAIR, COLOR_WHITE, COLOR_GREY);
	init_pair(HIGHLIGHT_EXPR_COLOR_PAR, COLOR_GREEN, COLOR_BLACK);
	init_pair(MUTED_COLOR_PAIR, COLOR_LIGHT_GREY, COLOR_BLACK);
	init_pair(AMBIGUOUS_COLOR_PAIR, COLOR_BLACK, COLOR_RED);
}

void setup_windows() {
	//Wird der stdsrc nicht minimal klein gemacht, verdeckt er den Rest
	wresize(stdscr, 0, 0);
	wnoutrefresh(stdscr);

	main_viewport = new scrollable(width - QUEUE_WIDTH - 1, height - HEADER_HEIGHT - FOOTER_HEIGHT - 1, 0,
			HEADER_HEIGHT);
	wbkgd(**main_viewport, COLOR_PAIR(STD_COLOR_PAIR));
	main_viewport->prepare_refresh();

	main_viewport_horizontal_center = main_viewport->get_width() / 2;
	main_viewport_vertical_center = main_viewport->get_height() / 2;

	footer = newwin(FOOTER_HEIGHT, width - QUEUE_WIDTH, height - FOOTER_HEIGHT, 0);
	wbkgd(footer, COLOR_PAIR(FOOTER_COLOR_PAIR));
	center_text_hor(footer, FOOTER_QUICK_ACTIONS_TEXT, 0);
	keypad(footer, TRUE);
	wnoutrefresh(footer);

	src_display = newwin(HEADER_HEIGHT, width - QUEUE_WIDTH, 0, 0);
	wbkgd(src_display, COLOR_PAIR(HEADER_COLOR_PAIR));
	mvwaddnstr(src_display, 0, getmaxx(src_display) / 2 - src_str_center, &src_str.elems[0], *src_str);
	wnoutrefresh(src_display);

	queue_display = new scrollable(QUEUE_WIDTH - 1, height - 1, width - QUEUE_WIDTH, 0);
	wbkgd(**queue_display, COLOR_PAIR(STD_COLOR_PAIR));
	queue_display->prepare_refresh();

	auto* popup_win = new scrollable(POPUP_WIDTH, POPUP_HEIGHT, main_viewport_horizontal_center - POPUP_WIDTH / 2,
			main_viewport_vertical_center - POPUP_HEIGHT / 2);
	wbkgd(**popup_win, COLOR_PAIR(POPUP_COLOR_PAIR));
	opers_popup = new popup(popup_win);

	auto* help_win = new scrollable(POPUP_WIDTH, POPUP_HEIGHT, main_viewport_horizontal_center - POPUP_WIDTH / 2,
			main_viewport_vertical_center - POPUP_HEIGHT / 2);
	wbkgd(**help_win, COLOR_PAIR(POPUP_COLOR_PAIR));
	help_popup = new popup(help_win);
	mvpaddstr(help_popup, 0, 0, "q: quit");
	mvpaddstr(help_popup, 1, 0, "n: go one step forward");
	mvpaddstr(help_popup, 2, 0, "p: go one step backward");
	mvpaddstr(help_popup, 3, 0, "o: show all operators");
	mvpaddstr(help_popup, 4, 0, "h: show this help");
	mvpaddstr(help_popup, 5, 0, "arrow-up: scroll archives/popup up");
	mvpaddstr(help_popup, 6, 0, "arrow-down: scroll archives/popup down");
	mvpaddstr(help_popup, 7, 0, "w: scroll queue up");
	mvpaddstr(help_popup, 8, 0, "s: scroll queue down");
	mvpaddstr(help_popup, 9, 0, "m [a-z]: set marker to the current event");
	mvpaddstr(help_popup, 10, 0, "' [a-z]: jump to marker");
	mvpaddstr(help_popup, 11, 0, "[0-9]+ <command>: execute command n times");

	popup_manager::the().insert(opers_popup, [&]() {
		prev_scrollable = current_scrollable;
		current_scrollable = **opers_popup;
	}, [&]() {
		current_scrollable = prev_scrollable;
		main_viewport->prepare_refresh();
	});
	popup_manager::the().insert(help_popup, {}, [&]() {
		main_viewport->prepare_refresh();
	});

}

/**
 * @return `true`, falls noch ein nächstes Event existiert hat,
 * sonst `false`
 */
bool step_n_events_forward(int n) {
	assert(n >= 0);

	for(int i = 0; i < n; ++i) {
		if(next_event_it == events.end())
			return false;
		++current_event_index;
		// Hatte das aktuelle event keine Auswirkung, wird direkt das Nächste ausgeführt
		if((*(next_event_it++))->exec() == event::did_nothing)
			--i;
	}

	return true;
}

/**
 * @return `true`, falls noch ein vorheriges Event existiert hat,
 * sonst `false`
 */
bool step_n_events_backward(int n) {
	assert(n >= 0);

	for(int i = 0; i < n; ++i) {
		if(next_event_it == events.begin())
			return false;
		--current_event_index;
		// Hatte das aktuelle event keine Auswirkung, wird direkt das Nächste ausgeführt
		if((*(--next_event_it))->undo() == event::did_nothing)
			--i;
	}

	return true;
}

int start_visualizer(const CH::str& source_string, int event_to_scip_to) {
	assert(event_to_scip_to >= 0);

	current_state = s_any_input;

	src_str = source_string;
	src_str_center = *src_str / 2;

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

	setup_windows();
	current_scrollable = main_viewport;

	next_event_it = events.begin();
	current_event_index = 0;

	step_n_events_forward(event_to_scip_to);

	wmove(src_display, 0, 0);
	wprintw(src_display, EVENT_COUNTER_TEXT, current_event_index);
	wnoutrefresh(src_display);

	wmove(footer, 0, 0);
	wprintw(footer, MULTIPLIER_TEXT, 1);

	doupdate();

	int multiplier = 0;
	while(int c = wgetch(footer)) {
		const auto& use_multiplier = [&multiplier]() {
			int multiplier_backup = multiplier;
			multiplier = 0;
			return std::max(1, multiplier_backup);
		};
		bool worked = true;

		if(current_state & s_wait_for_marker) {
			if(c == KEY_ESCAPE) {
				current_state = s_any_input;
				goto skip_with_refresh;
			}
			const auto& is_invalid_marker_name = [&c]() { return !(c >= 'a' && c <= 'z'); };
			if(current_state & s_read_marker) {
				if(is_invalid_marker_name() || markers.find(c) == markers.end()) {
					worked = false;
					goto skip_without_refresh;
				}
				int marked_event_index = markers.at(c);
				if(marked_event_index > current_event_index)
					step_n_events_forward(marked_event_index - current_event_index);
				else if(marked_event_index < current_event_index)
					step_n_events_backward(current_event_index - marked_event_index);
			}else if(current_state & s_create_marker) {
				if(is_invalid_marker_name() || markers.find(c) != markers.end()) {
					worked = false;
					goto skip_without_refresh;
				}
				markers.try_emplace(c, current_event_index);
			}
			current_state = s_any_input;
			goto skip_with_refresh;
		}else if(current_state == s_any_input) {
			if(c == 'q')
				break;
			if(c == 'h') {
				popup_manager::the().toggle(help_popup);
				multiplier = 0; // multiplier zurücksetzen
			}else if(c == 'o') {
				popup_manager::the().toggle(opers_popup);
				multiplier = 0; // multiplier zurücksetzen
			}else if(c == 'n')
				worked = step_n_events_forward(use_multiplier());
			else if(c == 'p')
				worked = step_n_events_backward(use_multiplier());
			else if(c == 'm') {
				current_state = s_wait_for_marker | s_create_marker;
				multiplier = 0; // multiplier zurücksetzen
			}else if(c == '\'') {
				current_state = s_wait_for_marker | s_read_marker;
				multiplier = 0; // multiplier zurücksetzen
			}else if(c == KEY_ARROW_DOWN)
				worked = current_scrollable->scroll_y(use_multiplier());
			else if(c == KEY_ARROW_UP)
				worked = current_scrollable->scroll_y(-use_multiplier());
			else if(c == 'w')
				worked = queue_display->scroll_y(-use_multiplier());
			else if(c == 's')
				worked = queue_display->scroll_y(use_multiplier());
			else if(c >= '0' && c <= '9') {
				multiplier = multiplier * 10 + (c - 48);
				goto skip_with_refresh;
			}else {
				worked = false;
				goto skip_without_refresh;
			}
			goto skip_with_refresh;
		}

		skip_with_refresh:
		wmove(src_display, 0, 0);
		wprintw(src_display, EVENT_COUNTER_TEXT, current_event_index);
		wnoutrefresh(src_display);

		werase(footer);
		if(current_state & s_wait_for_marker)
			center_text_hor(footer, FOOTER_WAIT_FOR_MARKER_TEXT, 0);
		else
			center_text_hor(footer, FOOTER_QUICK_ACTIONS_TEXT, 0);
		wmove(footer, 0, 0);
		wprintw(footer, MULTIPLIER_TEXT, std::max(1, multiplier));
		wnoutrefresh(footer);

		// das muss als Letztes vor `doupdate` ausgeführt werden, damit das popup,
		// falls es denn angezeigt wird, nicht vom rest überlagert wird
		if(popup_manager::the().is_one_popup_shown())
			popup_manager::the().prepare_refresh_for_shown_popups();

		//`doupdate` hier nötig, da alle aufgerufenen Funktionen in der Regel nu `wnoutrefresh` verwenden
		doupdate();

		skip_without_refresh:
		if(!worked) {
			beep();
			flash();
		}
	}

	handleSignal(0);
	return 0;
}