#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "popup.hpp"

class popup_manager {
public:
	using callback = std::function<void()>;

	explicit popup_manager(int expected_size = 0);

	void insert(popup* popup, const std::optional<callback>& show_callback = {},
			const std::optional<callback>& hide_callback = {});

	bool toggle(popup* popup) const;
	void show(popup* popup) const;
	void hide(popup* popup) const;

	void prepare_refresh_for_shown_popups();

	bool is_one_popup_shown() const;

private:
	std::vector<popup*> m_popups;
	std::unordered_map<popup*, callback> m_show_callbacks;
	std::unordered_map<popup*, callback> m_hide_callbacks;
};