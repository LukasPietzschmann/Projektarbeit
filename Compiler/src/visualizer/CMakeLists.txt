add_library(Visualizer main.hpp main.cpp archive.cpp utils.hpp utils.cpp windows.hpp layouting.cpp layouting.hpp archive_change_listener.hpp event.cpp event.hpp event.cpp main.hpp events.hpp constants.hpp)
link_libraries(Visualizer flxc)

include_directories(
		../libCH
		../flxc
)