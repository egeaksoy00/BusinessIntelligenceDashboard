cmake_minimum_required(VERSION 3.16)
project(BusinessIntelligenceDashboard LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(raylib)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_filters tests/test_filters.c src/filters.c src/data.c)
    target_include_directories(test_filters PRIVATE src)
    add_test(NAME dashboard_filters COMMAND test_filters)
endif()

add_executable(bi_dashboard src/main.c src/dashboard.c src/data.c src/filters.c)
target_link_libraries(bi_dashboard PRIVATE raylib)
if(MSVC)
    target_compile_options(bi_dashboard PRIVATE /W4)
else()
    target_compile_options(bi_dashboard PRIVATE -Wall -Wextra -Wpedantic)
    if(UNIX)
        target_link_libraries(bi_dashboard PRIVATE m)
    endif()
endif()
