
cmake_minimum_required(VERSION 2.6)
project(GBL)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c11")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Og -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories(${GBL_SOURCE_DIR}/include)

set(SOURCES
        src/flatview.cc
)

add_library(GBL ${SOURCES})

enable_testing()

set(TEST_LIBS
                      GBL
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

set(TESTS
    tests/data_test.cc
    tests/flatview_test.cc
    tests/netlist_test.cc
    tests/testing.cc
)
add_executable(tests.bin ${TESTS})
target_link_libraries(tests.bin ${TEST_LIBS})
add_test(test tests.bin)

