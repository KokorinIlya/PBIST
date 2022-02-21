project(tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(run_tests.out 
    all_tests.cpp
)
target_link_libraries(run_tests.out pthread gtest gtest_main)
