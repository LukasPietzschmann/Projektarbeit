#pragma once

#include <vector>
#include "archive.hpp"

class archive;

class archive_change_listener {
public:
	virtual void notify_dimensions_changed(archive& a) const = 0;
};