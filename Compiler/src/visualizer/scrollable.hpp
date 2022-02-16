#pragma once

#include <algorithm>
#include <cassert>
#include <ncurses.h>
#include <seq.ch>

#include "constants.hpp"
#include "window_like.hpp"

#define mvsaddstr(scrollable, y, x, str) (scrollable)->add_n_str((str), (x), (y))

class scrollable : public window_like<WINDOW> {
public:
	scrollable(uint32_t width, uint32_t height, uint32_t x_start, uint32_t y_start);
	~scrollable();

	void scroll_y(int delta);
	uint32_t get_width() const;

	void add_n_str(const CH::str& str, int x, int y) override;
	void del_line(int x, int y) override;
	void clear() override;
	void prepare_refresh() const override;

private:
	uint32_t m_width;
	uint32_t m_screen_height;
	uint32_t m_x_start;
	uint32_t m_y_start;
	uint32_t m_scroll_amount {0};
	uint32_t m_content_height {0};
	uint32_t m_content_start_y {0};
};