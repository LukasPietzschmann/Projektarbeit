#include "popup_manager.hpp"

popup_manager::popup_manager(int expected_size) {
	m_popups.reserve(expected_size);
}

void popup_manager::insert(popup* popup) {
	assert(popup != nullptr);
	m_popups.push_back(popup);
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
		p->hide();
	}
	popup->show();
}

void popup_manager::hide(popup* popup) const {
	assert(popup != nullptr);
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