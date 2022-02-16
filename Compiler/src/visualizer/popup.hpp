#pragma once

#include <panel.h>

#include "scrollable.hpp"
#include "window_like.hpp"

#define mvpaddstr(popup, y, x, str) (popup)->add_n_str((str), (x), (y))

class popup : public window_like<scrollable> {
	friend class popup_manager;

public:
	explicit popup(scrollable* window);
	~popup() override;

	bool is_currently_shown() const;

	void add_n_str(const CH::str& str, int x, int y) override;
	void del_line(int x, int y) override;
	void clear() override;
	void prepare_refresh() const override;

private:
	void show();
	void hide();
	PANEL* m_panel;
	bool m_is_currently_shown {false};
};