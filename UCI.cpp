#include "UCI.hpp"
#include "notation.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

void UCI::loop()
{
    std::string cmd, token;

    while (token != "quit" && std::getline(std::cin, cmd)){
        std::istringstream iss(cmd);

        token.clear();
        iss >> std::skipws >> token;

        if (token == "uci") {
            std::string uciInfo = "id name Bagatto\nid author Claudio Raciti\noption name Hash type spin default 1 min 1 max 128\nuciok";
            std::cout << uciInfo << std::endl;
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "ucinewgame"){
            mEngine.setPos(STARTPOS);
        }
        else if (token == "setoptions"){
            std::string idName, name, idValue, value;
            iss >> idName >> name >> idValue >> value;
            int memory = stoi(value);
            if (idName == "name" && name == "Hash" && idValue == "value") {
                if (memory >= 1 && memory <= 128) mEngine.resizeTT(memory);
                else std::cout << "value out of bounds" << std::endl;
            }
        }
        else if (token == "position") {
            std::string fen;
            iss >> token;

            if (token == "startpos") {
                fen = STARTPOS;
                iss >> token;
            } 
            else if (token == "fen") {
                while (iss >> token && token != "moves"){
                    fen += token + " ";
                }
            }
            else {
                std::cout << "Invalid argument for 'position': " << token << std::endl;
                continue;
            }

            mEngine.setPos(fen);
            while (iss >> token) mEngine.makeMove(token);
        }
        else if (token == "go") {
            go(iss);
        }
        else if (token == "stop" || token == "quit"){
            mEngine.stopSearch();
        }
    }
}

void UCI::go(std::istringstream& tIss)
{
    std::string token;
    SearchLimits limits;
    limits.timestart = now();


    while(tIss >> token){
        if (token == "infinite")
            limits.infinite = true; 
        else if (token == "wtime")
            tIss >> limits.time[white];
        else if (token == "btime")
            tIss >> limits.time[black];
        else if (token == "winc")
            tIss >> limits.inc[white];
        else if (token == "binc")
            tIss >> limits.inc[black];
        else if (token == "movestogo")
            tIss >> limits.movestogo;
        else if (token == "depth")
            tIss >> limits.depth;
        else if (token == "nodes")
            tIss >> limits.nodes;
        else if (token == "movetime") tIss >> limits.movetime; 
    }
    
    mEngine.goSearch(limits);
}