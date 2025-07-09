#pragma once

#include "Engine.hpp"
#include "notation.hpp"
#include <sstream>

class UCI
{
private:
    Engine mEngine;
public:
    UCI() = default;
    ~UCI() = default;

    void loop();
private:
    void go(std::istringstream& iss);
};