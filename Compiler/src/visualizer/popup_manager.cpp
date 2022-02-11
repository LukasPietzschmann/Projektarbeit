#include "popup_manager.hpp"

popup_manager::popup_manager(int expected_size) {
	m_popups.reserve(expected_size);
}

void popup_manager::insert(popup* popup, const std::optional<popup_manager::callback>& show_callback,
		const std::optional<popup_manager::callback>& hide_callback) {
	assert(popup != nullptr);
	m_popups.push_back(popup);
	m_show_callbacks.try_emplace(popup, show_callback.has_value() ? *show_callback : []() {});
	m_hide_callbacks.try_emplace(popup, hide_callback.has_value() ? *hide_callback : []() {});
}

bool popup_manager::toggle(popup* popup) const {
	assert(popup != nullptr);
	if(popup->is_currently_shown()) {
		hide(popup);
		return false;
	}
	show(popup);
	return true;
}

void popup_manager::show(popup* popup) const {
	assert(popup != nullptr);
	for(auto* p: m_popups) {
		if(p == popup)
			continue;
		hide(p);
	}
	m_show_callbacks.at(popup)();
	popup->show();
}

void popup_manager::hide(popup* popup) const {
	assert(popup != nullptr);
	m_hide_callbacks.at(popup)();
	popup->hide();
}

void popup_manager::prepare_refresh_for_shown_popups() {
	for(const auto* p: m_popups) {
		if(p->m_is_currently_shown)
			p->prepare_refresh();
	}
}

bool popup_manager::is_one_popup_shown() const {
	return std::any_of(m_popups.begin(), m_popups.end(), [](const popup* p) {
		return p->m_is_currently_shown;
	});
}