#pragma once

#include <algorithm>
#include <algorithm>
#include <cassert>
#include <vector>

#include "popup.hpp"

class popup_manager {
public:
	explicit popup_manager(int expected_size = 0);

	void insert(popup* popup);

	bool toggle(popup* popup) const;
	void show(popup* popup) const;
	void hide(popup* popup) const;

	void prepare_refresh_for_shown_popups();

	bool is_one_popup_shown() const;

private:
	std::vector<popup*> m_popups;
};