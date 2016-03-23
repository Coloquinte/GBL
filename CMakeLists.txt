
cmake_minimum_required(VERSION 2.6)
project(GBL)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c11")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Og -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")

include_directories(${GBL_SOURCE_DIR}/include/public)
include_directories(${GBL_SOURCE_DIR}/include/private)

set(DATA_TEST
  tests/data_test.cc
)

enable_testing()

add_executable(data_test.bin ${DATA_TEST})

add_test(data_test ${CMAKE_BINARY_DIR}/data_test.bin)

