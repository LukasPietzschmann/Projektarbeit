#pragma once

#include <vector>

#include "archive.hpp"

class archive;

class archive_change_listener {
public:
	virtual void notify_dimensions_changed(archive&) const = 0;
	virtual void notify_visuals_changed(archive&) const = 0;
};