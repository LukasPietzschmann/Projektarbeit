add_subdirectory(flxc)
add_subdirectory(visualizer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Readline)
find_package(Curses)

include(FetchContent)
FetchContent_Declare(
		cxxopts
		GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
		GIT_TAG c74846a891b3cc3bfa992d588b1295f528d43039 # v3.0.0
)
FetchContent_MakeAvailable(cxxopts)

include_directories(
    ./libCH
    ./flxc
    ./visualizer
     ${Readline_INCLUDE_DIR}
)

add_executable(main main)
target_link_libraries(main flxc Visualizer cxxopts)

if(CURSES_FOUND)
	include_directories(${CURSES_INCLUDE_DIR})
	target_link_libraries(main ${CURSES_LIBRARIES})
	add_compile_definitions(HAS_CURSES_LIB)
endif(CURSES_FOUND)

if (READLINE_FOUND)
	target_link_libraries(main ${Readline_LIBRARY})
	add_compile_definitions(HAS_READLINE_LIB)
endif (READLINE_FOUND)