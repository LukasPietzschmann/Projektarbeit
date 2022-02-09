#pragma once

#include <panel.h>

#define mvpaddnstr(popup, y, x, str, len) mvwaddnstr(**popup, y + 1, x + 1, str, len)

class popup {
public:
	explicit popup(WINDOW* window);
	~popup();

	void show();
	void hide();
	bool toggle();
	bool is_currently_shown() const;

	WINDOW* operator*();

private:
	PANEL* m_panel;
	WINDOW* m_origin_win;
	bool m_is_currently_shown {false};
};