cmake_minimum_required(VERSION 2.8.9)
project (cboe)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
add_executable(main main.cpp)

add_executable(main_ut main.cpp)
target_link_libraries(main_ut ${GTEST_LIBRARIES} pthread)
set_target_properties(main_ut PROPERTIES COMPILE_FLAGS "-D__UNITTEST__")

enable_testing()
add_test(NAME mytest COMMAND main_ut)

