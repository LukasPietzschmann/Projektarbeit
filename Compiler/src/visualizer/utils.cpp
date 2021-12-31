#include "utils.hpp"

void center_text_hor(WINDOW* window, const std::string& str, uint32_t y_coordinate) {
	assert(window != nullptr);
	uint32_t x_coordinate = (getmaxx(window) / 2) - (str.size() / 2);
	mvwaddnstr(window, y_coordinate, x_coordinate, str.c_str(), str.size());
	wnoutrefresh(window);
}