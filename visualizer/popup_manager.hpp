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

	static popup_manager& the();

	popup_manager(const popup_manager&) = delete;
	popup_manager(popup_manager&&) noexcept = default;
	popup_manager& operator=(const popup_manager&) = delete;
	popup_manager& operator=(popup_manager&&) noexcept = default;

	void insert(popup* popup, const std::optional<callback>& show_callback = {},
			const std::optional<callback>& hide_callback = {});

	bool toggle(popup* popup) const;
	void show(popup* popup) const;
	void hide(popup* popup) const;

	void prepare_refresh_for_shown_popups();

	bool is_one_popup_shown() const;

private:
	popup_manager() = default;

	std::vector<popup*> m_popups;
	std::unordered_map<popup*, callback> m_show_callbacks;
	std::unordered_map<popup*, callback> m_hide_callbacks;
};