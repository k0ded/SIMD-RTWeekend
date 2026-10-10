#pragma once

#include "SIMD/SMFloatAll.h"
#include "SIMD/SMVector3.h"
#include "SIMD/SMVector2.h"

using Float = simdsse::SMFloat;
using Vec3 = SAE::SMVector3<Float>;
using Vec2 = SAE::SMVector2<Float>;
using Mask = Float::Mask;