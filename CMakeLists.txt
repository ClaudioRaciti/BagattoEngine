cmake_minimum_required(VERSION 3.10.0)
project(engine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4)

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob3 /GL /DNDEBUG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O2 -g -fno-inline)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -flto -march=native -DNDEBUG)
    endif()
endif()

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