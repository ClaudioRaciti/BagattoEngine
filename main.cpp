#include "Engine.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <thread>

int main(){
    Engine engine;

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "uci") {
            std::cout << "id name Bagatto" << std::endl;
            std::cout << "id author Claudio Raciti" << std::endl;
            std::cout << "option name Hash type spin default 1 min 1 max 128" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (input == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (input == "ucinewgame"){
            engine.setPos(STARTPOS);
        }
        else if (input.rfind("setoption", 0) == 0){
            std::istringstream iss(input);
            std::string token, idName, name, idValue, value;
            iss >> token >> idName >> name >> idValue >> value;
            int memory = stoi(value);
            if (idName == "name" && name == "Hash" && idValue == "value") {
                if (memory >= 1 && memory <= 128) engine.resizeTT(memory);
                else std::cout << "value out of bounds" << std::endl;
            }
        }
        else if (input.rfind("position", 0) == 0) {
            std::istringstream iss(input);
            std::string token, fen;
            std::vector<std::string> moves;
            iss >> token; // "position"
            std::string positionType;
            iss >> positionType;

            if (positionType == "startpos") {
                engine.setPos(STARTPOS);
                std::string next;
                if (iss >> next && next == "moves") {
                    std::string move;
                    while (iss >> move) {
                        engine.makeMove(move);
                    }
                }
            } 
            else if (positionType == "fen") {
                fen.clear();
                for (int i = 0; i < 6; i++) {
                    std::string part;
                    iss >> part;
                    fen += part + " ";
                }
                engine.setPos(fen);

                std::string next;
                if (iss >> next && next == "moves") {
                    std::string move;
                    while (iss >> move) {
                        engine.makeMove(move);
                    }
                }
            }
        }
        else if (input.rfind("go", 0) == 0) {
            engine.goSearch(99);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            engine.stopSearch();
        }
        else if (input == "stop"){
            engine.stopSearch();
        }
        else if (input == "quit") {
            engine.stopSearch();
            break;
        }
    }
}