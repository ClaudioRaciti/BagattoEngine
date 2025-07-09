#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <chrono>

// Time variables
using TimePoint = std::chrono::milliseconds::rep;  // A value in milliseconds
static_assert(sizeof(TimePoint) == sizeof(int64_t), "TimePoint should be 64 bits");
inline TimePoint now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// Search data
struct SearchLimits
{
    bool infinite = false;
    uint64_t nodes = 0ULL;
    int depth = 0, movestogo = 0;
    TimePoint movetime = 0, timestart = 0;
    TimePoint time[2], inc[2];
};

// Bit TWiddling functions
constexpr void wrapNort (uint64_t &bitBoard) {bitBoard <<= 8;}
constexpr void wrapSout (uint64_t &bitBoard) {bitBoard >>= 8;}
constexpr void wrapEast (uint64_t &bitBoard) {constexpr uint64_t mask = uint64_t(0x7f7f7f7f7f7f7f7f); bitBoard &= mask; bitBoard <<= 1;}
constexpr void wrapWest (uint64_t &bitBoard) {constexpr uint64_t mask = uint64_t(0xfefefefefefefefe); bitBoard &= mask; bitBoard >>= 1;}
constexpr uint64_t cpyWrapNort (uint64_t bitBoard) {wrapNort(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapSout (uint64_t bitBoard) {wrapSout(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapEast (uint64_t bitBoard) {wrapEast(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapWest (uint64_t bitBoard) {wrapWest(bitBoard); return bitBoard;}
int bitScanReverse (uint64_t bitBoard);
int bitScanForward (uint64_t bitBoard);

// Population count functions
void initPopCount256();
int popCount(uint64_t bitBoard);

// String manipulation functions
std::vector<std::string> split (const std::string &string, char delim);
bool is_number(const std::string& s);

// Hashing functions
// MurmurHash3Mixer 
constexpr uint64_t hash64(uint64_t key){key = ((key >> 33) ^ key) * 0xff51afd7ed558ccd; key = ((key >> 33) ^ key) * 0xc4ceb9fe1a85ec53; return key ^ (key >>33);}
constexpr uint32_t hash32(uint32_t key){key = ((key >> 16) ^ key) * 0x45d9f3b; key = ((key >> 16) ^ key) * 0x45d9f3b; return key ^ (key >>16);}