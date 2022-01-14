#pragma once

#include <algorithm>
#include <ncurses.h>
#include <cassert>

class scrollable {
public:
	scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start);
	~scrollable();

	void scroll_y(int delta);

	void project_here(WINDOW* win);

	void clear();
	void refresh();

	WINDOW* operator*() const;

private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_x_start;
	uint32_t m_y_start;
	WINDOW* m_pad;
	int m_scroll_y {0};
	int m_max_y {0};
};