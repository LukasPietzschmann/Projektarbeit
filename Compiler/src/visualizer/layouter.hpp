#pragma once

#include <unordered_map>

#include "archive.hpp"
#include "archive_change_listener.hpp"
#include "windows.hpp"

class layouter : public archive_change_listener {
public:
	static layouter& the();

	layouter(const layouter&) = delete;
	layouter(layouter&&) noexcept = default;
	layouter& operator=(const layouter&) = delete;
	layouter& operator=(layouter&&) = default;

	void register_new_archive(archive& a);
	void unregister_archive(archive& a);

	void notify_dimensions_changed(archive& archive_to_layout) const override;
	void notify_visuals_changed(archive& archive_to_rerender) const override;

private:
	layouter() = default;
};