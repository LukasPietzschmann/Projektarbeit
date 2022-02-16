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

void layouter::notify_dimensions_changed(archive&) const {
	const auto& layout = [](archive& archive_to_layout) {
		const auto has_intersections = [&archive_to_layout](const archive::rect& rect_to_test) {
			return std::any_of(arch_windows.begin(), arch_windows.end(),
					[&archive_to_layout, &rect_to_test](const archive& a) {
						if(!a.is_layouted || a == archive_to_layout)
							return false;
						return a.intersects_with(rect_to_test);
					});
		};

		int desired_x_coord = std::max(0,
				(int) (main_viewport_horizontal_center - src_str_center + archive_to_layout.get_pos_in_src()) -
						(int) archive_to_layout.get_divider_x_pos());

		uint32_t y_coordinate = 0;

		while(has_intersections({(uint32_t) desired_x_coord, y_coordinate, archive_to_layout.get_width(),
				archive_to_layout.get_height()}))
			++y_coordinate;

		archive_to_layout.set_y_start(y_coordinate);
		archive_to_layout.set_x_start(desired_x_coord);
	};

	for(archive& a: arch_windows)
		a.is_layouted = false;

	for(archive& a: arch_windows) {
		layout(a);
		a.is_layouted = true;
	}

	main_viewport->clear();
	for(auto& window: arch_windows)
		window.render();

	main_viewport->prepare_refresh();
}