// Vectorization
// 3 = AVX512 ~21.89% of computers support AVX512   // 16 floats (allows speedups, but lacks support)
// 2 = AVX256 ~94.84% of computers support AVX2     // 8 floats
// 1 = SSE4.2 ~97.93% of computers support SSE4.1   // 4 floats
// 0 = No vectorization
#ifndef VECTORIZATION
#define VECTORIZATION 1 // SAFE DEFAULT. 
#endif

#if VECTORIZATION == 3
#define AVX512
#define NAMESPACE simdavx512
#elif VECTORIZATION == 2
#define AVX2
#define NAMESPACE simdavx
#elif VECTORIZATION == 1
#define SSE
#define NAMESPACE simdsse
#else
#define NAMESPACE nosimd
#endif

#if VECTORIZATION == 3
#include <zmmintrin.h>
#endif

#ifdef AVX2
#include <immintrin.h>
#endif

#ifdef SSE
#include <immintrin.h>
#include <smmintrin.h> // SSE4.2 intrinsics
#endif

#ifndef SIMDSHARED_NAMESPACE
#define SIMDSHARED_NAMESPACE
#include <cassert>
#include <corecrt_math.h>
namespace simdshared
{
    enum class CompareFunctions
    {
        Equal = 0x00,         /* Equal (ordered, nonsignaling)                    */
        Less = 0x11,          /* Less-than (ordered, signaling)                   */
        LessEqual = 0x12,     /* Less-than-or-equal (ordered, signaling)          */
        Greater = 0x1E,       /* Greater-than (ordered, nonsignaling)             */
        GreaterEqual = 0x1D,  /* Greater-than-or-equal (ordered, nonsignaling)    */
        NotEqual = 0x0C      /* Not-equal (ordered, non-signaling)               */

    };
}
#endif

namespace NAMESPACE
{
    class SMMask
    {
#ifdef AVX512
        using value_type = __mmask16;
        using flt_type = __m512;
        using int_type = __m512i;
#elif defined(AVX2)
        using value_type = __m256;
        using flt_type = __m256;
        using int_type = __m256i;
#elif defined(SSE)
        using value_type = __m128;
        using flt_type = __m128;
        using int_type = __m128i;
#else
        using value_type = bool;
        using flt_type = float;
        using int_type = int;
#endif
        value_type mask;

    public:
        SMMask() : mask(value_type()) {}
        SMMask(const value_type& aMask) : mask(aMask) {}

        __forceinline int CountHigh()
        {
            constexpr int num_to_bits[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

#ifdef AVX512
            return num_to_bits[mask & 0xF] + num_to_bits[(mask >> 4) & 0xF] + num_to_bits[(mask >> 8) & 0xF] + num_to_bits[(mask >> 12) & 0xF];
#elif defined(AVX2)
            int msk = _mm256_movemask_ps(mask);
            return num_to_bits[msk & 0xF] + num_to_bits[(msk >> 4) & 0xF];
#elif defined(SSE)
            return num_to_bits[_mm_movemask_ps(mask)]; // Only 4 bits
#else
            return mask ? 1 : 0;
#endif
        }

        __forceinline bool AllSet()
        {
#ifdef AVX512
            return mask == 0xFFFF;
#elif defined(AVX2)
            return _mm256_testc_si256(_mm256_castps_si256(mask), _mm256_set1_epi32(-1));
#elif defined(SSE)
            return _mm_movemask_ps(mask) == 0xF;
#else
            return mask;
#endif
        }

        __forceinline bool NoneSet()
        {
#ifdef AVX512
            return mask == 0;
#elif defined(AVX2)
            return _mm256_testz_si256(_mm256_castps_si256(mask), _mm256_set1_epi32(-1));
#elif defined(SSE)
            return _mm_movemask_ps(mask) == 0;
#else
            return !mask;
#endif
        }

        SMMask& operator&=(const SMMask& aMask)
        {
#ifdef AVX512
            mask &= aMask.mask;
#elif defined(AVX2)
            mask = _mm256_and_ps(mask, aMask.mask);
#elif defined(SSE)
            mask = _mm_and_ps(mask, aMask.mask);
#else
            mask = mask && aMask.mask;
#endif
            return *this;
        }

        SMMask operator&(const SMMask& aMask) const
        {
            SMMask m = *this;
            m &= aMask;
            return m;
        }

        SMMask& operator|=(const SMMask& aMask)
        {
#ifdef AVX512
            mask |= aMask.mask;
#elif defined(AVX2)
            mask = _mm256_or_ps(mask, aMask.mask);
#elif defined(SSE)
            mask = _mm_or_ps(mask, aMask.mask);
#else
            mask = mask || aMask.mask;
#endif
            return *this;
        }

        SMMask operator|(const SMMask& aMask) const
        {
            SMMask m = *this;
            m |= aMask;
            return m;
        }

        SMMask operator~()
        {
#ifdef AVX512
            return _knot_mask16(mask);
#elif defined(AVX2)
            return _mm256_xor_ps(mask, _mm256_castsi256_ps(_mm256_set1_epi32(-1)));
#elif defined(SSE)
            return _mm_xor_ps(mask, _mm_castsi128_ps(_mm_set1_epi32(-1)));
#else
            return !mask;
#endif
        }

        SMMask ifelse(SMMask aSuccess, SMMask aFail)
        {
#ifdef AVX512
            return (mask & aSuccess.mask) | (~mask & aFail.mask);
#elif defined(AVX2) || defined(SSE)
            return SMMask(replace_if(aFail.mask, aSuccess.mask));
#else
            return mask ? aSuccess : aFail;
#endif
        }

        flt_type add_if(flt_type dst, flt_type aAdd) const
        {
#ifdef AVX512
            return _mm512_mask_add_ps(dst, mask, dst, aAdd);
#elif defined(AVX2)
            return _mm256_add_ps(dst, _mm256_and_ps(mask, aAdd));
#elif defined(SSE)
            return _mm_add_ps(dst, _mm_and_ps(mask, aAdd));
#else
            return mask ? dst + aAdd : dst;
#endif
        }

        flt_type replace_if(flt_type fail, flt_type success) const
        {
#ifdef AVX512
            return _mm512_mask_blend_ps(mask, fail, success);
#elif defined(AVX2)
            return _mm256_blendv_ps(fail, success, mask);
#elif defined(SSE)
            return _mm_blendv_ps(fail, success, mask);
#else
            return mask ? success : fail;
#endif
        }
    };

    class SMFloat
    {
    private:
#ifdef AVX512
        using value_type = __m512;
        using int_type = __m512i;
#elif defined(AVX2)
        using value_type = __m256;
        using int_type = __m256i;
#elif defined(SSE)
        using value_type = __m128;
        using int_type = __m128i;
#else
        using value_type = float;
        using int_type = int;
#endif

        union
        {
            value_type v;
            int_type i;
        };

        // Private since this is only valid in very specific scenarios...
        __forceinline SMFloat(const int_type& t) : i(t) {}

#ifdef AVX512
        __forceinline SMFloat(const int t) : i(_mm512_set1_epi32(t)) {}
#elif defined(AVX2)
        __forceinline SMFloat(const int t) : i(_mm256_set1_epi32(t)) {}
#elif defined(SSE)
        __forceinline SMFloat(const int t) : i(_mm_set1_epi32(t)) {}
#endif

    public:
        using Mask = SMMask;

#ifdef AVX512
        SMFloat(const float t = 0.0f) : v(_mm512_set1_ps(t)) {}
        SMFloat(const value_type& t) : v(t) {}
#elif defined(AVX2)
        SMFloat(const float t = 0.0f) : v(_mm256_set1_ps(t)) {}
        SMFloat(const value_type& t) : v(t) {}
#elif defined(SSE)
        SMFloat(const float t = 0.0f) : v(_mm_set1_ps(t)) {}
        SMFloat(const value_type& t) : v(t) {}
#else
        SMFloat(const value_type& t = 0.0f) : v(t) {}
#endif

        __forceinline SMFloat MultiplyAdd(const SMFloat& aMultiplyFloat, const SMFloat& aAddFloat) const;
        __forceinline SMFloat Abs() const;

        __forceinline SMFloat Tan() const;
        __forceinline SMFloat ATan() const;

        __forceinline SMFloat Sin() const;
        __forceinline SMFloat ASin() const;

        __forceinline SMFloat Cos() const;
        __forceinline SMFloat ACos() const;

        __forceinline SMFloat Sqrt() const;
        __forceinline SMFloat InvSqrt() const;

        __forceinline SMFloat Min(SMFloat aFloat) const;
        __forceinline SMFloat Max(SMFloat aFloat) const;

        __forceinline SMFloat Pow(SMFloat aFloat) const;

        __forceinline SMFloat& operator+=(const SMFloat& aOtherFloat);
        [[nodiscard]] __forceinline SMFloat operator+(const SMFloat& aOtherFloat) const;
        __forceinline SMFloat& operator-=(const SMFloat& aOtherFloat);
        [[nodiscard]] __forceinline SMFloat operator-(const SMFloat& aOtherFloat) const;

        [[nodiscard]] __forceinline SMFloat operator-() const;

        __forceinline SMFloat& operator*=(const SMFloat& aOtherFloat);
        __forceinline SMFloat& operator/=(const SMFloat& aOtherFloat);

        __forceinline SMFloat& operator&=(const SMFloat& aOtherFloat);
        __forceinline SMFloat& operator^=(const SMFloat& aOtherFloat);

        __forceinline SMFloat& operator++();
        [[nodiscard]] __forceinline SMFloat operator++(int);

        __forceinline SMFloat& operator--();
        [[nodiscard]] __forceinline SMFloat operator--(int);

        [[nodiscard]] __forceinline Mask operator<(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Less)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Less)));
#elif defined(SSE)
            // Fallback to SSE
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Less)));
#else
            return v < aOther.v;
#endif
        }

        [[nodiscard]] __forceinline Mask operator<=(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::LessEqual)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::LessEqual)));
#elif defined(SSE)
            // Fallback to SSE
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::LessEqual)));
#else
            return v <= aOther.v;
#endif
        }
        [[nodiscard]] __forceinline Mask operator>(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Greater)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Greater)));
#elif defined(SSE)
            // Fallback to SSE
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Greater)));
#else
            return v > aOther.v;
#endif
        }
        [[nodiscard]] __forceinline Mask operator>=(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::GreaterEqual)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::GreaterEqual)));
#elif defined(SSE)
            // Fallback to SSE
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::GreaterEqual)));
#else
            return v >= aOther.v;
#endif
        }
        [[nodiscard]] __forceinline Mask operator==(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Equal)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Equal)));
#elif defined(SSE)
            // Fallback to AVX
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::Equal)));
#else
            return v == aOther.v;
#endif
        }

        [[nodiscard]] __forceinline Mask operator!=(const SMFloat& aOther) const
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::NotEqual)));
#elif defined(AVX2)
            // Fallback to AVX2
            return SMMask(_mm256_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::NotEqual)));
#elif defined(SSE)
            // Fallback to AVX
            return SMMask(_mm_cmp_ps(v, aOther.v, static_cast<const int>(simdshared::CompareFunctions::NotEqual)));
#else
            return v != aOther.v;
#endif
        }

        [[nodiscard]] bool IsUniform()
        {
#ifdef AVX512
            // Use AVX-512 Foundation and VL intrinsics
            return SMMask(_mm512_cmp_ps_mask(v, _mm512_shuffle_ps(v, v, 1), static_cast<const int>(simdshared::CompareFunctions::Equal))).AllSet();
#elif defined(AVX2)
            // Fallback to AVX2
            const __m128i lane0 = _mm256_castsi256_si128(_mm256_castps_si256(v));
            const __m128i tmp = _mm_shuffle_epi8(lane0, _mm_setzero_si128());
            const __m256i populated_0th_byte = _mm256_set_m128i(tmp, tmp);
            const __m256i eq = _mm256_cmpeq_epi8(_mm256_castps_si256(v), populated_0th_byte);

            return ((unsigned)_mm256_movemask_epi8(eq) == 0xffffffff);
#elif defined(SSE)
            // Fallback to AVX
        	const __m128i rotated = _mm_alignr_epi8(_mm_castps_si128(v), _mm_castps_si128(v), 1);
            const __m128i eq = _mm_cmpeq_epi8(_mm_castps_si128(v), rotated);

            return ((unsigned short)_mm_movemask_epi8(eq) == 0xffff);
#else
            return true;
#endif
        }

        [[nodiscard]] __forceinline SMFloat addif(const Mask& mask, const SMFloat& aAdd);

        [[nodiscard]] __forceinline static SMFloat ifelse(const Mask& mask, const SMFloat& success, const SMFloat& fail);

        [[nodiscard]] constexpr static unsigned ChannelCount()
        {
            return sizeof(value_type) / sizeof(float);  // NOLINT(bugprone-sizeof-expression)
        }

        __forceinline static SMFloat Zero()
        {
            static auto zero = SMFloat(0.0f);
            return zero;
        }

    private:
        static void ddxHelper(float& aLesser, float& aLarger)
        {
            float intermediate = aLarger - aLesser;
            aLesser = intermediate;
            aLarger = intermediate;
        }
    public:

        SMFloat ddx() const
        {
            assert(ChannelCount() >= 4); // Not valid on types with less than 4 channels

            SMFloat result = *this;
            ddxHelper(result[0], result[1]);
            ddxHelper(result[2], result[3]);

            if constexpr (ChannelCount() >= 8)
            {
                ddxHelper(result[4], result[5]);
                ddxHelper(result[6], result[7]);
            }

            if constexpr (ChannelCount() >= 12)
            {
                ddxHelper(result[8], result[9]);
                ddxHelper(result[10], result[11]);
            }

            if constexpr (ChannelCount() >= 16)
            {
                ddxHelper(result[12], result[13]);
                ddxHelper(result[14], result[15]);
            }
            return result;
        }

        SMFloat ddy() const
        {
            assert(ChannelCount() >= 4); // Not valid on types with less than 4 channels

            SMFloat result = *this;
            ddxHelper(result[0], result[2]);
            ddxHelper(result[1], result[3]);

            if constexpr (ChannelCount() >= 8)
            {
                ddxHelper(result[4], result[6]);
                ddxHelper(result[5], result[7]);
            }

            if constexpr (ChannelCount() >= 12)
            {
                ddxHelper(result[8], result[10]);
                ddxHelper(result[9], result[11]);
            }

            if constexpr (ChannelCount() >= 16)
            {
                ddxHelper(result[12], result[14]);
                ddxHelper(result[13], result[15]);
            }
            return result;
        }

        float& operator[](size_t aIndex);
        const float& operator[](size_t aIndex) const;
    };

    __forceinline float& SMFloat::operator[](size_t aIndex)
    {
        if (aIndex >= ChannelCount())
        {
            assert(false && "Index out of bounds");
        }

        // Not redundant as v might be one of the vector types.
        // ReSharper disable once CppRedundantCastExpression
        return reinterpret_cast<float*>(&v)[aIndex];
    }

    __forceinline const float& SMFloat::operator[](size_t aIndex) const
    {
        if (aIndex >= ChannelCount())
        {
            assert(false && "Index out of bounds");
        }

        // Not redundant as v might be one of the vector types.
        // ReSharper disable once CppRedundantCastExpression
        return reinterpret_cast<const float*>(&v)[aIndex];
    }

    __forceinline SMFloat SMFloat::MultiplyAdd(const SMFloat& aMultiplyFloat, const SMFloat& aAddFloat) const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_fmadd_ps(v, aMultiplyFloat.v, aAddFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_fmadd_ps(v, aMultiplyFloat.v, aAddFloat.v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_fmadd_ps(v, aMultiplyFloat.v, aAddFloat.v);
#else
        return v * aMultiplyFloat.v + aAddFloat.v;
#endif
    }

    __forceinline SMFloat SMFloat::Abs() const
    {
        SMFloat f = *this;
        f &= SMFloat(INT_MAX); // Only has the sign bit not set (same location for floats) this breaks strict aliasing...
        return f;
    }

    __forceinline SMFloat SMFloat::Tan() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_tan_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_tan_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_tan_ps(v);
#else
        return tanf(v);
#endif
    }

    __forceinline SMFloat SMFloat::ATan() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_atan_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_atan_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_atan_ps(v);
#else
        return atanf(v);
#endif
    }

    __forceinline SMFloat SMFloat::Sin() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_sin_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_sin_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_sin_ps(v);
#else
        return sinf(v);
#endif
    }

    __forceinline SMFloat SMFloat::ASin() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_asin_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_asin_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_asin_ps(v);
#else
        return asinf(v);
#endif
    }

    __forceinline SMFloat SMFloat::Cos() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_cos_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_cos_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_cos_ps(v);
#else
        return cosf(v);
#endif
    }

    __forceinline SMFloat SMFloat::ACos() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_acos_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_acos_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_acos_ps(v);
#else
        return acosf(v);
#endif
    }

    __forceinline SMFloat SMFloat::Sqrt() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_sqrt_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_sqrt_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_sqrt_ps(v);
#else
        return sqrtf(v);
#endif
    }

    __forceinline SMFloat SMFloat::InvSqrt() const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_invsqrt_ps(v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_invsqrt_ps(v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_invsqrt_ps(v);
#else
        return 1.0f / sqrtf(v);
#endif
    }

    __forceinline SMFloat SMFloat::Min(SMFloat aFloat) const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_min_ps(v, aFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_min_ps(v, aFloat.v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_min_ps(v, aFloat.v);
#else
        return v < aFloat.v ? v : aFloat.v;
#endif
    }

    __forceinline SMFloat SMFloat::Max(SMFloat aFloat) const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_max_ps(v, aFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_max_ps(v, aFloat.v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_max_ps(v, aFloat.v);
#else
        return v > aFloat.v ? v : aFloat.v;
#endif
    }

    inline SMFloat SMFloat::Pow(SMFloat aFloat) const
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        return _mm512_pow_ps(v, aFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        return _mm256_pow_ps(v, aFloat.v);
#elif defined(SSE)
        // Fallback to AVX
        return _mm_pow_ps(v, aFloat.v);
#else
        return powf(v, aFloat.v);
#endif
    }

    __forceinline SMFloat& SMFloat::operator+=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_add_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_add_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to AVX
        v = _mm_add_ps(v, aOtherFloat.v);
#else
        v += aOtherFloat.v;
#endif

        return *this;
    }

    __forceinline SMFloat SMFloat::operator+(const SMFloat& aOtherFloat) const
    {
        SMFloat f = *this;
        f += aOtherFloat;
        return f;
    }

    __forceinline SMFloat& SMFloat::operator-=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_sub_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_sub_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to SSE
        v = _mm_sub_ps(v, aOtherFloat.v);
#else
        v += aOtherFloat.v;
#endif
        return *this;
    }

    __forceinline SMFloat SMFloat::operator-(const SMFloat& aOtherFloat) const
    {
        SMFloat f = *this;
        f -= aOtherFloat;
        return f;
    }

    __forceinline SMFloat SMFloat::operator-() const
    {
        return SMFloat(0.0f) - *this;
    }

    __forceinline SMFloat& SMFloat::operator*=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_mul_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_mul_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to SSE
        v = _mm_mul_ps(v, aOtherFloat.v);
#else
        v += aOtherFloat.v;
#endif
        return *this;
    }

    

    __forceinline SMFloat& SMFloat::operator/=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_div_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_div_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to SSE
        v = _mm_div_ps(v, aOtherFloat.v);
#else
        v += aOtherFloat.v;
#endif
        return *this;
    }

    __forceinline SMFloat& SMFloat::operator&=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_and_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_and_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to SSE
        v = _mm_and_ps(v, aOtherFloat.v);
#else
        i &= aOtherFloat.i;
#endif
        return *this;
    }

    __forceinline SMFloat& SMFloat::operator^=(const SMFloat& aOtherFloat)
    {
#ifdef AVX512
        // Use AVX-512 Foundation and VL intrinsics
        v = _mm512_xor_ps(v, aOtherFloat.v);
#elif defined(AVX2)
        // Fallback to AVX2
        v = _mm256_xor_ps(v, aOtherFloat.v);
#elif defined(SSE)
        // Fallback to SSE
        v = _mm_xor_ps(v, aOtherFloat.v);
#else
        i ^= aOtherFloat.i;
#endif
        return *this;
    }

    __forceinline SMFloat& SMFloat::operator++()
    {
        *this = *this + 1.0f;
        return *this;
    }

    __forceinline SMFloat SMFloat::operator++(int)
    {
        SMFloat f = SMFloat(*this);
        ++*this;
        return f;
    }

    __forceinline SMFloat& SMFloat::operator--()
    {
        *this = *this - 1.0f;
        return *this;
    }

    __forceinline SMFloat SMFloat::operator--(int)
    {
        SMFloat f = SMFloat(*this);
        --*this;
        return f;
    }

    __forceinline SMFloat SMFloat::ifelse(const SMMask& mask, const SMFloat& success, const SMFloat& fail)
    {
        return mask.replace_if(fail.v, success.v);
    }

    __forceinline SMFloat SMFloat::addif(const SMMask& mask, const SMFloat& aAdd)
    {
        return mask.add_if(v, aAdd.v);
    }
}

__forceinline NAMESPACE::SMFloat operator*(const NAMESPACE::SMFloat& aFloat, const NAMESPACE::SMFloat& aOtherFloat)
{
    NAMESPACE::SMFloat f = aFloat;
    f *= aOtherFloat;
    return f;
}

__forceinline NAMESPACE::SMFloat operator/(const NAMESPACE::SMFloat& aFloat, const NAMESPACE::SMFloat& aOtherFloat)
{
    NAMESPACE::SMFloat f = aFloat;
    f /= aOtherFloat;
    return f;
}

__forceinline NAMESPACE::SMFloat operator&(const NAMESPACE::SMFloat& aFloat, const NAMESPACE::SMFloat& aOtherFloat)
{
    NAMESPACE::SMFloat f = aFloat;
    f &= aOtherFloat;
    return f;
}

__forceinline NAMESPACE::SMFloat operator^(const NAMESPACE::SMFloat& aFloat, const NAMESPACE::SMFloat& aOtherFloat)
{
    NAMESPACE::SMFloat f = aFloat;
    f ^= aOtherFloat;
    return f;
}

__forceinline NAMESPACE::SMFloat operator*(float lhs, const NAMESPACE::SMFloat& rhs)
{
    return rhs * lhs;
}

__forceinline NAMESPACE::SMFloat operator+(float lhs, const NAMESPACE::SMFloat& rhs)
{
    return rhs + lhs;
}

__forceinline NAMESPACE::SMFloat operator-(float lhs, const NAMESPACE::SMFloat& rhs)
{
    return NAMESPACE::SMFloat(lhs) - rhs;
}

#undef NAMESPACE
#undef SSE
#undef AVX2
#undef AVX512