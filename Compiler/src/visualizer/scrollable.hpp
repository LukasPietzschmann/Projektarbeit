#pragma once

#include <algorithm>
#include <ncurses.h>
#include <cassert>
#include <seq.ch>

#define mvsaddstr(scrollable, y, x, str) scrollable->add_string(str, x, y)
#define mvsaddch(scrollable, y, x, chr) scrollable->add_char(chr, x, y)

class scrollable {
public:
	scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start);
	~scrollable();

	void scroll_y(int delta);

	void add_string(const CH::str& string, int x, int y);
	void add_char(char c, int x, int y);
	void del_line(int x, int y);

	void clear();
	void prepare_refresh();

	uint32_t get_width() const;
	uint32_t get_height() const;

	WINDOW* operator*() const;

private:
	uint32_t m_width;
	uint32_t m_screen_height;
	uint32_t m_x_start;
	uint32_t m_y_start;
	WINDOW* m_pad;
	uint32_t m_scroll_amount {0};
	uint32_t m_content_height {0};
	uint32_t m_content_start_y {0};
};