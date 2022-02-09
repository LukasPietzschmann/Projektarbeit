#pragma once

#include <panel.h>

#include "scrollable.hpp"

#define mvpaddstr(popup, y, x, str) mvsaddstr((**popup), y, x, str)

class popup {
public:
	explicit popup(scrollable* window);
	~popup();

	void show();
	void hide();
	bool toggle();
	bool is_currently_shown() const;

	void prepare_refresh() const;

	scrollable* operator*();

private:
	PANEL* m_panel;
	scrollable* m_origin_win;
	bool m_is_currently_shown {false};
};