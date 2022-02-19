project(MOSTflexiPL)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

option(USE_ALTERNATIVE_PANEL_HEADER OFF)
if (${USE_ALTERNATIVE_PANEL_HEADER})
	add_compile_definitions(PANEL_HEADER=<panel.h>)
else ()
	add_compile_definitions(PANEL_HEADER=<ncurses/panel.h>)
endif ()

add_subdirectory(src)

# **********************************************
# *********   GOOGLE Test **********************
# **********************************************

option(WITH_TESTS "Include and compile (Google) tests" ON)

if(WITH_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
            URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
        )
        # For Windows: Prevent overriding the parent
        # project's compiler/linker settings.
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)

        add_subdirectory(tests)
        enable_testing()
endif(WITH_TESTS)
