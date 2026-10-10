#pragma once
#include <limits>

#include "SIMDUsings.h"
#include "SIMD/SMFloatAll.h"

class Interval {
public:
    Float min, max;

    Interval() : min(+std::numeric_limits<float>::infinity()), max(-std::numeric_limits<float>::infinity()) {} // Default interval is empty
    Interval(Float min, Float max) : min(min), max(max) {}

    Float size() const {
        return max - min;
    }

    Mask contains(Float x) const {
        return min <= x & max > x;
    }

    Mask surrounds(Float x) const {
        return min < x & max >= x;
    }

    Float clamp(Float x) const {
        return x.Max(min).Min(max);
    }

    static const Interval empty, universe;
};

inline const Interval Interval::empty = Interval(+std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
inline const Interval Interval::universe = Interval(-std::numeric_limits<float>::infinity(), +std::numeric_limits<float>::infinity());