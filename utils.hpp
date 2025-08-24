#pragma once

#include <cstdint>
#include <chrono>
#include <cassert>

#ifdef _MSC_VER
#include <intrin.h>
#elif !(defined(__GNUC__) || defined(__clang__))
#include <array>
#endif

/**
 * @brief Returns index of the least significant active (1) bit
 * 
 * @param bitBoard Board rapresentation as unsigned long long
 * @return int Index of the LSB
 */
inline int bitScanForward(uint64_t bitBoard){
    assert (bitBoard != 0);
#if defined (_MSC_VER) 
    unsigned long index;
    _BitScanForward64(&index, bitBoard);
    return static_cast<int>(index);
#elif  defined (__GNUC__) || defined (__clang__)
    return __builtin_ctzll(bitBoard);
#else
    //Reference table for bitscans
    static constexpr std::array<int,64> index64 = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    static constexpr uint64_t deBrujin64 = uint64_t(0x03f79d71b4cb0a89);
    return index64[((bitBoard ^ (bitBoard-1)) * deBrujin64) >> 58];
#endif
}

/**
 * @brief Returns index of the most significant active (1) bit
 * 
 * @param bitBoard Board rapresentation as unsigned long long
 * @return int Index of the MSB
 */
inline int bitScanReverse(uint64_t bitBoard){
    assert (bitBoard != 0);
#if defined (_MSC_VER) 
    unsigned long index;
    _BitScanReverse64(&index, bitBoard);
    return static_cast<int>(index);
#elif  defined (__GNUC__) || defined (__clang__)
    return __builtin_clzll(bitBoard);
#else
    // Reference table for bitscans
    static constexpr std::array<int,64> index64 = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
    };
    static constexpr uint64_t deBrujin64 = uint64_t(0x03f79d71b4cb0a89);
    bitBoard |= bitBoard >> 1;
    bitBoard |= bitBoard >> 2;
    bitBoard |= bitBoard >> 4;
    bitBoard |= bitBoard >> 8;
    bitBoard |= bitBoard >> 16;
    bitBoard |= bitBoard >> 32;
    return index64[(bitBoard * deBrujin64) >> 58];
#endif
}

/**
 * @brief Returns the total number of active (1) bits
 * 
 * @param bitBoard Board rapresentation as unsigned long long
 * @return int active bits count
 */
inline int popCount(uint64_t bitBoard){
#if defined (_MSC_VER)
    return static_cast<int>(__popcnt64(bitBoard));
#elif  defined (__GNUC__) || defined (__clang__)
    return __builtin_popcountll(bitBoard);
#else
    static constexpr std::array<int, 256> _popCount256 = [] {
        std::array<int, 256> a {};
        a[0] = 0;
        for(int i = 1; i < 256; i ++)
            a[i] = a[i / 2] + (i & 1);
        return a;
    } ();
    const uint8_t* p = reinterpret_cast<uint8_t *>(&bitBoard);
    return  _popCount256[p[0]] + _popCount256[p[1]] + _popCount256[p[2]] + _popCount256[p[3]] +
            _popCount256[p[4]] + _popCount256[p[5]] + _popCount256[p[6]] + _popCount256[p[7]];
#endif
}

// Bit TWiddling functions
constexpr void wrapNort (uint64_t &bitBoard) {bitBoard <<= 8;}
constexpr void wrapSout (uint64_t &bitBoard) {bitBoard >>= 8;}
constexpr void wrapEast (uint64_t &bitBoard) {
    constexpr uint64_t mask = uint64_t(0x7f7f7f7f7f7f7f7f); 
    bitBoard &= mask; 
    bitBoard <<= 1;
}
constexpr void wrapWest (uint64_t &bitBoard) {
    constexpr uint64_t mask = uint64_t(0xfefefefefefefefe);
    bitBoard &= mask;
    bitBoard >>= 1;
}
constexpr uint64_t cpyWrapNort (uint64_t bitBoard) {wrapNort(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapSout (uint64_t bitBoard) {wrapSout(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapEast (uint64_t bitBoard) {wrapEast(bitBoard); return bitBoard;}
constexpr uint64_t cpyWrapWest (uint64_t bitBoard) {wrapWest(bitBoard); return bitBoard;}

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

