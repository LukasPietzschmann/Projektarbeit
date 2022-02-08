add_library(Visualizer main.hpp main.cpp archive.cpp utils.hpp utils.cpp windows.hpp layouting.cpp layouting.hpp archive_change_listener.hpp event.cpp event.hpp event.cpp main.hpp events.hpp constants.hpp expr_queue.cpp expr_queue.hpp scrollable.hpp scrollable.cpp popup.hpp popup.cpp oper_store.hpp oper_store.cpp)
link_libraries(Visualizer flxc)

include_directories(
		../libCH
		../flxc
)