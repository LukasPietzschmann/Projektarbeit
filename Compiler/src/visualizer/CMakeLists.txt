add_library(Visualizer visualizer.cpp archive.cpp utils.cpp layouter.cpp event.cpp event.cpp expr_queue.cpp scrollable.cpp
		popup.cpp oper_store.cpp popup_manager.cpp expr_repr.cpp)
target_link_libraries(Visualizer flxc)

include_directories(
		../libCH
		../flxc
)