cmake_minimum_required(VERSION 3.5.0)
project(engine VERSION 0.1.0 LANGUAGES C CXX)

add_executable(engine main.cpp notation.hpp)

include(CTest)
enable_testing()

