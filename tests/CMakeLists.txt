project(tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(run_tests.out 
    test_tree_building.cpp
)
target_link_libraries(run_tests.out pthread cilkrts gtest gtest_main)