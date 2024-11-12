cmake_minimum_required(VERSION 3.16.3)

set(This tests)
set(Files 
    test.cpp
)

add_executable(${This} ${Files})

target_link_libraries(${This} 
    gtest_main
    testing
    ${JPEG_LIBRARIES}
)

add_test(
    NAME ${This}
    COMMAND ${This}
)