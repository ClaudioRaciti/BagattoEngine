#pragma once 

#include <cstdint>

class TTValue
{
private:
    uint32_t mData; 
public:
    TTValue() : mData{0U} {}
    TTValue(int16_t tScore, int tDepth, int tNodeType) : 
        mData{uint32_t(tScore & 0xffff) | uint32_t((tDepth & 0xff) << 16) | uint32_t((tNodeType & 0xf) << 24)} {}
    ~TTValue() = default;
    inline bool operator==(const TTValue &t_other) const {return mData == t_other.mData;}

    constexpr int16_t score() const {return int16_t(mData & 0xffff);}
    constexpr int nodeType()  const {return (mData >> 24) & 0xf;}
    constexpr int depth()     const {return (mData >> 16) & 0xff;}
};