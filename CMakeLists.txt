
cmake_minimum_required(VERSION 2.6)
project(GBL)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c11")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Og -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")

include_directories(${GBL_SOURCE_DIR}/include)

set(SOURCES
        src/flatview.cc
)

add_library(GBL ${SOURCES})

enable_testing()

set(DATA_TEST tests/data_test.cc)
add_executable(data_test.bin ${DATA_TEST})
target_link_libraries(data_test.bin GBL)
add_test(data_test data_test.bin)

set(NETLIST_TEST tests/netlist_test.cc)
add_executable(netlist_test.bin ${NETLIST_TEST})
target_link_libraries(netlist_test.bin GBL)
add_test(netlist_test netlist_test.bin)

set(FLATVIEW_TEST tests/flatview_test.cc)
add_executable(flatview_test.bin ${FLATVIEW_TEST})
target_link_libraries(flatview_test.bin GBL)
add_test(flatview_test flatview_test.bin)
