cmake_minimum_required(VERSION 3.5)

ADD_EXECUTABLE(test.exe main.cpp)
SET(CMAKE_CXX_COMPILER g++-7)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -isystem /home/sehe/custom/boost \
    -Wall -Wextra -pedantic \
    -std=c++1z \
    -g -O2 -pthread -march=native")
