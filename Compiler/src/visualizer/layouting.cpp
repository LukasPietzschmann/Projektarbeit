#include "layouting.hpp"

layouter& layouter::the() {
	static layouter instance;
	return instance;
}

void layouter::register_new_cat(archive& c) {
	c.register_as_listener(this);
}

void layouter::unregister_cat(archive& c) {
	c.unregister_as_listener(this);
}

void layouter::notify_dimensions_changed(archive& a) const {
	const auto& layout = [](archive& a) {
		const auto has_intersections = [&a](const archive::rect& rect_to_test) {
			for(const archive& window: arch_windows) {
				if(window == a)
					continue;
				if(window.intersects_with(rect_to_test))
					return true;
			}
			return false;
		};

		int desired_x_coord = a.get_pos_in_src() - (a.get_width() / 2);
		if(desired_x_coord < 1)
			desired_x_coord = 1;
		uint32_t y_coordinate = 0;

		while(has_intersections({a.get_x_start(), y_coordinate, a.get_width(), a.get_height()}))
			++y_coordinate;

		a.set_y_start(y_coordinate);
		a.set_x_start(desired_x_coord);
	};

	for(archive& a: arch_windows)
		layout(a);

	wclear(stdscr);
	wnoutrefresh(stdscr);

	for(auto& window: arch_windows)
		window.render();

	doupdate();
}