#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

#include "SIMDUsings.h"
#include "SIMD/SMVector3.h"


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

// Utility Functions

inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0f;
}

inline float random_float_i() {
    thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    thread_local std::mt19937 generator;
    return distribution(generator);
}

inline float random_float_i(float min, float max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_float_i();
}

inline Float* GetArray()
{
    static Float* twister = new Float[256 * 256];
    return twister;
}

inline Float random_float() {
    thread_local unsigned short index = 0;
    return GetArray()[index++];
}

inline void Random_Initialize()
{
    for (int i = 0; i < 256; ++i)
    {
        for (int j = 0; j < 256; ++j)
        {
            for (unsigned k = 0; k < Float::ChannelCount(); ++k)
            {
                GetArray()[i * 256 + j] = random_float_i();
            }
        }
    }
}

inline Float random_float(const Float& min, const Float& max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_float();
}

inline Float randomFloat()
{
    Float result = random_float();
    return result;
}

inline Float randomFloat(const Float& min, const Float& max)
{
    Float result = random_float(min, max);
    return result;
}

inline Vec3 randomVec()
{
    return { randomFloat(), randomFloat(), randomFloat() };
}

inline Vec3 randomVec(Float min, Float max)
{
    return { randomFloat(min, max), randomFloat(min, max), randomFloat(min, max) };
}

inline Vec3 random_unit_vector() {

    Mask finishedMask{};
    Vec3 result{};
    while (!finishedMask.AllSet()) {
        auto p = randomVec(-1.0f, 1.0f);
        auto lensq = p.LengthSqr();

        auto withinUsableRange = lensq >= 1.175494351e-20F;
        auto withinUnitVector = lensq <= 1.0f;
        auto validResult = withinUnitVector & withinUsableRange;

        result = Vec3::ifelse(validResult, p / lensq.Sqrt(), result);
        finishedMask |= validResult;
    }
    return result;
}

inline Vec3 random_on_hemisphere(const Vec3& normal) {
    Vec3 on_unit_sphere = random_unit_vector();
    return Vec3::ifelse(on_unit_sphere.Dot(normal) > 0.0f, on_unit_sphere, -on_unit_sphere);
}

inline Vec3 random_in_unit_disk() {
    Mask finishedMask{};
    Vec3 result{};

    while (!finishedMask.AllSet()) {
        auto p = Vec3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
        auto lensq = p.LengthSqr();
        auto validResult = lensq < 1.0f;

        result = Vec3::ifelse(validResult, p, result);
        finishedMask = finishedMask | validResult;
    }

    return result;
}

inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0f * v.Dot(n) * n;
}

inline Vec3 refract(const Vec3& uv, const Vec3& n, const Float& etai_over_etat) {
    auto cos_theta = (-uv).Dot(n).Min(1.0f);
    Vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -((1.0f - r_out_perp.LengthSqr()).Abs().Sqrt()) * n;
    return r_out_perp + r_out_parallel;
}

// Common Headers

#include "interval.h"
#include "Ray.h"
#include "SIMD/SMVector2.h"