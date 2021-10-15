include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

set(PROJECT_LIBS
    gtest_main
    flxc
)

add_executable(tests tests)

target_link_libraries(tests ${PROJECT_LIBS})

include(GoogleTest)
gtest_discover_tests(tests)

option(AUTORUN_TESTS "Automatically run all tests after successful compilation" ON)
if(AUTORUN_TESTS)
    add_custom_command(TARGET tests COMMAND ../bin/tests POST_BUILD)
endif(AUTORUN_TESTS)
