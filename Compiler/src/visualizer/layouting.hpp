#pragma once

#include "archive.hpp"
#include "archive_change_listener.hpp"
#include "windows.hpp"
#include <unordered_map>

class layouter : public archive_change_listener{
public:
	static layouter& the();

	layouter(const layouter&) = delete;
	layouter(layouter&&) noexcept = default;
	layouter& operator=(const layouter&) = delete;
	layouter& operator=(layouter&&) = default;

	void register_new_cat(archive& c);
	void unregister_cat(archive& c);

	void notify_dimensions_changed(archive& a) const override;

private:
	layouter() = default;
};