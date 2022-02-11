#include "main.hpp"

WINDOW* footer;
WINDOW* src_display;
scrollable* queue_display;
scrollable* main_viewport;
popup* opers_popup;
popup* help_popup;

popup_manager* p_manager;

scrollable* current_scrollable;

int main_viewport_center;
int src_str_center;
int src_str_len;
int width;
int height;
CH::str src_str;

std::vector<archive> arch_windows;
std::vector<event*> events;
event_iterator next_event_it;
uint64_t current_event_index;

std::map<char, int> markers;

int current_state;

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

void setup_windows() {
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
	center_text_hor(footer, "q: quit    n: next    p: previous    o: toggle opers    h: help", 0);
	keypad(footer, TRUE);

	src_display = newwin(HEADER_HEIGHT, width - QUEUE_WIDTH, 0, 0);
	wbkgd(src_display, COLOR_PAIR(HEADER_COLOR_PAIR));
	mvwaddstr(src_display, 0, getmaxx(src_display) / 2 - *src_str / 2, &src_str.elems[0]);

	queue_display = new scrollable(QUEUE_WIDTH - 1, height - 1, width - QUEUE_WIDTH, 0);
	wbkgd(**queue_display, COLOR_PAIR(QUEUE_COLOR_PAIR));
	queue_display->prepare_refresh();

	auto* popup_win = new scrollable(POPUP_WIDTH, POPUP_HEIGHT, main_viewport_center - POPUP_WIDTH / 2, 5);
	wbkgd(**popup_win, COLOR_PAIR(POPUP_COLOR_PAIR));
	opers_popup = new popup(popup_win);

	auto* help_win = new scrollable(POPUP_WIDTH, POPUP_HEIGHT, main_viewport_center - POPUP_WIDTH / 2, 5);
	wbkgd(**help_win, COLOR_PAIR(POPUP_COLOR_PAIR));
	help_popup = new popup(help_win);
	help_win->add_string("arrow-up: scroll up", 0, 0);
	help_win->add_string("arrow-down: scroll down", 0, 1);
	help_win->add_string("s: switch window to scroll in", 0, 2);
	help_win->add_string("m [a-z]: set marker to the current event", 0, 3);
	help_win->add_string("' [a-z]: jump to marker", 0, 4);
	help_win->add_string("[0-9]+ <command>: execute command n times", 0, 5);

	p_manager = new popup_manager(2);
	p_manager->insert(opers_popup);
	p_manager->insert(help_popup);

	wnoutrefresh(src_display);
	wnoutrefresh(footer);
}

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

	setup_windows();
	current_scrollable = main_viewport;

	next_event_it = events.begin();
	current_event_index = 0;

	step_n_events_forward(event_to_scip_to);

	wmove(src_display, 0, 0);
	wprintw(src_display, "Event: %d", current_event_index);
	wnoutrefresh(src_display);

	doupdate();

	int multiplier = 0;
	scrollable* prev_scrollable;
	while(int c = wgetch(footer)) {
		const auto& use_multiplier = [&multiplier]() {
			int multiplier_backup = multiplier;
			multiplier = 0;
			return std::max(1, multiplier_backup);
		};
		bool worked = true;

		if(current_state == s_wait_for_scrollable_selection) {
			if(c == KEY_ESCAPE) {
				current_state = s_any_input;
				goto skip_with_refresh;
			}
			if(c == 'a')
				current_scrollable = main_viewport;
			else if(c == 'b')
				current_scrollable = queue_display;
			else if(c == 'o' && opers_popup->is_currently_shown())
				current_scrollable = **opers_popup;
			else {
				worked = false;
				goto skip_without_refresh;
			}
			current_state = s_any_input;
			goto skip_with_refresh;
		}else if(current_state & s_wait_for_marker) {
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
				if(!p_manager->toggle(help_popup))
					main_viewport->prepare_refresh();
			}else if(c == 'o') {
				if(p_manager->toggle(opers_popup)) {
					prev_scrollable = current_scrollable;
					current_scrollable = **opers_popup;
				}else { // wenn das popup geschlossen wird, sollten die archive neu gerendert werden
					main_viewport->prepare_refresh();
					current_scrollable = prev_scrollable;
				}
			}else if(c == 'n')
				worked = step_n_events_forward(use_multiplier());
			else if(c == 'p')
				worked = step_n_events_backward(use_multiplier());
			else if(c == 'm')
				current_state = s_wait_for_marker | s_create_marker;
			else if(c == '\'')
				current_state = s_wait_for_marker | s_read_marker;
			else if(c == 's')
				current_state = s_wait_for_scrollable_selection;
			else if(c == KEY_ARROW_DOWN)
				current_scrollable->scroll_y(use_multiplier());
			else if(c == KEY_ARROW_UP)
				current_scrollable->scroll_y(-use_multiplier());
			else if(c >= '0' && c <= '9') {
				multiplier = multiplier * 10 + (c - 48);
				goto skip_without_refresh;
			}else {
				worked = false;
				goto skip_without_refresh;
			}
			goto skip_with_refresh;
		}

		skip_with_refresh:
		wmove(src_display, 0, 0);
		wprintw(src_display, "Event: %d", current_event_index);
		wnoutrefresh(src_display);

		werase(footer);
		if(current_state & s_wait_for_marker)
			center_text_hor(footer, "Input marker or press <esc>", 0);
		else if(current_state == s_wait_for_scrollable_selection)
			center_text_hor(footer, "Input Scrollable-Selector (a: archives b: queue o: opers) or press <esc>", 0);
		else
			center_text_hor(footer, "q: quit    n: next    p: previous    o: toggle opers    h: help", 0);
		wnoutrefresh(footer);

		// das muss als letztes vor `doupdate` ausgeführt werden, damit das popup,
		// falls es denn angezeigt wird, nicht vom rest überlagert wird
		if(p_manager->is_one_popup_shown())
			p_manager->prepare_refresh_for_shown_popups();

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