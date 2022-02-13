#pragma once

#include <panel.h>

#include "scrollable.hpp"

#define mvpaddstr(popup, y, x, str) mvsaddstr(**(popup), (y), (x) + 1, (str))

class popup {
	friend class popup_manager;

public:
	explicit popup(scrollable* window);
	~popup();

	bool is_currently_shown() const;

	void prepare_refresh() const;

	scrollable* operator*();

private:
	void show();
	void hide();
	/**
	 * @return `true`, falls das popup geöffnet wurde,
	 * sonst `false`
	 */
	PANEL* m_panel;
	scrollable* m_origin_win;
	bool m_is_currently_shown {false};
};