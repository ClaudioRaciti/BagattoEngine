#pragma once

#include "Engine.hpp"
#include <sstream>

class UCI
{
private:
    Engine mEngine;
public:
    UCI() = default;
    ~UCI() = default;

    /**
     * @brief Input parser that controls the engine, as per the UCI specifications 
     */
    void loop();
private:
    void go(std::istringstream& iss);
};