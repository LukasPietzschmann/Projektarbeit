#pragma once

#include <panel.h>

#define mvpaddstr(popup, y, x, str) mvwaddstr(**popup, y + 1, x + 1, str)
#define mvpaddch(popup, y, x, chr) mvwaddch(**popup, y + 1, x + 1, chr)

class popup {
public:
	explicit popup(WINDOW* window);
	~popup();

	void show();
	void hide();
	bool toggle();
	bool is_currently_shown();

	WINDOW* operator*();

private:
	PANEL* m_panel;
	WINDOW* m_origin_win;
	bool m_is_currently_shown {false};
};