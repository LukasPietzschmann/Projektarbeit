add_subdirectory(flxc)

include_directories(
    ./libCH
    ./flxc
)

add_executable(main main)
target_link_libraries(main flxc)
