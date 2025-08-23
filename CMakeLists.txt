cmake_minimum_required(VERSION 3.5.0)
project(engine VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wpedantic -Wextra -O3")

add_executable(
    engine 
    main.cpp 
    notation.hpp 
    Board.hpp
    Board.cpp
    Move.hpp 
    Move.cpp 
    utils.hpp 
    MoveGenerator.hpp
    MoveGenerator.cpp
    MagicBitboards.hpp
    MagicBitboards.cpp
    Debugger.hpp
    Debugger.cpp
    TT.hpp
    TT.cpp
    evaluation.hpp
    evaluation.cpp
    Engine.hpp
    Engine.cpp
    Zobrist.hpp
    Zobrist.cpp
    UCI.hpp
    UCI.cpp
)

include(CTest)
enable_testing()

