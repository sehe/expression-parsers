cmake_minimum_required(VERSION 3.5)

ADD_EXECUTABLE(test.exe main.cpp test.cpp operators/impl.cpp report_error.cpp)
SET(CMAKE_CXX_COMPILER g++-7)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Wall -Wextra -pedantic \
    -std=c++1z \
    -g -O2 -pthread -march=native")
