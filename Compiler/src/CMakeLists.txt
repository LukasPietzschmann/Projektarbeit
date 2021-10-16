add_subdirectory(flxc)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Readline)
include_directories()

include_directories(
    ./libCH
    ./flxc
     ${Readline_INCLUDE_DIR}
)

add_executable(main main)
target_link_libraries(main flxc)

if (READLINE_FOUND)
	target_link_libraries(main ${Readline_LIBRARY})
	add_compile_definitions(HAS_READLINE_LIB)
endif (READLINE_FOUND)