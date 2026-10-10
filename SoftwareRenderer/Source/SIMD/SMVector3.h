#pragma once
#include <cassert>

#include "SMFloatAll.h"

namespace SAE
{
	template <typename T>
	class SMVector3
	{
	public:
		SMVector3() = default;
		SMVector3(const T& aXYZ);
		SMVector3(const T& aX, const T& aY, const T& aZ) : x(aX), y(aY), z(aZ) {}

		// Copy constructor (compiler generated)
		SMVector3(const SMVector3& aVector) = default;

		// Assignment operator (compiler generated)
		SMVector3& operator=(const SMVector3& aVector3) = default;

		// Destructor (compiler generated)
		~SMVector3() = default;

		/*void Load(const CU::Vector3f& aVec, unsigned aChannelIndex)
		{
			x[aChannelIndex] = aVec.x;
			y[aChannelIndex] = aVec.y;
			z[aChannelIndex] = aVec.z;
		}*/

		/*CU::Vector3f Store(unsigned aChannelIndex)
		{
			return {
				x[aChannelIndex],
				y[aChannelIndex],
				z[aChannelIndex]
			};
		}*/

		T& operator[](size_t aIndex);
		const T& operator[](size_t aIndex) const;

		// Returns a negated copy of the vector
		SMVector3 operator-() const;
		T::Mask operator==(const SMVector3& aOther) const;

		// Returns the squared length of the vector
		T LengthSqr() const;

		// Returns the length of the vector
		T Length() const;

		// Returns 1 / length of the vector
		T InvLength() const;

		// Sets the length of the vector to the provided distance
		void SetLength(T aLength);

		void Normalize();

		T Dot(const SMVector3& aVector) const;
		SMVector3 Cross(const SMVector3& aVector) const;

		SMVector3 ddx() const;
		SMVector3 ddy() const;
		SMVector3 Abs() const;
		SMVector3 GetNormalized() const;

		T::Mask NearZero() const
		{
			// Return true if the vector is close to zero in all dimensions.
			float s = 1e-7f;
			return (x.Abs() < s) & (y.Abs() < s) & (z.Abs() < s);
		}

		SMVector3 Min(const SMVector3& aOther);
		SMVector3 Max(const SMVector3& aOther);

		static SMVector3 ifelse(const T::Mask& aMask, const SMVector3& aTrue, const SMVector3& aFalse);

		[[nodiscard]] constexpr static unsigned long long ChannelCount()
		{
			return T::ChannelCount();
		}


		SMVector3& operator+=(const SMVector3& aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}

		SMVector3 operator+(const SMVector3& aFactor) const
		{
			SMVector3 f = *this;
			f += aFactor;
			return f;
		}

		SMVector3& operator-=(const SMVector3& aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}

		SMVector3 operator-(const SMVector3& aFactor) const
		{
			SMVector3 f = *this;
			f -= aFactor;
			return f;
		}

		SMVector3& operator*=(const SMVector3& aFactor)
		{
			x *= aFactor.x;
			y *= aFactor.y;
			z *= aFactor.z;
			return *this;
		}

		SMVector3 operator*(const SMVector3& aFactor) const
		{
			SMVector3 f = *this;
			f *= aFactor;
			return f;
		}

		SMVector3& operator*=(T aFactor)
		{
			x *= aFactor;
			y *= aFactor;
			z *= aFactor;
			return *this;
		}

		SMVector3 operator*(T aFactor) const
		{
			SMVector3 f = *this;
			f *= aFactor;
			return f;
		}

		SMVector3& operator/=(const SMVector3& aFactor)
		{
			x /= aFactor.x;
			y /= aFactor.y;
			z /= aFactor.z;
			return *this;
		}

		SMVector3 operator/(const SMVector3& aFactor) const
		{
			SMVector3 f = *this;
			f /= aFactor;
			return f;
		}

		SMVector3& operator/=(T aFactor)
		{
			x /= aFactor;
			y /= aFactor;
			z /= aFactor;
			return *this;
		}

		SMVector3 operator/(T aFactor) const
		{
			SMVector3 f = *this;
			f /= aFactor;
			return f;
		}

	private:
		T x, y, z;
	};

	template <typename T>
	SMVector3<T>::SMVector3(const T& aXYZ) : x(aXYZ), y(aXYZ), z(aXYZ)
	{
	}

	template <typename T>
	T& SMVector3<T>::operator[](size_t aIndex)
	{
		assert(aIndex < 3);
		return (&x)[aIndex];
	}

	template <typename T>
	const T& SMVector3<T>::operator[](size_t aIndex) const
	{
		return (&x)[aIndex];
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::operator-() const
	{
		return { -x, -y, -z };
	}

	template <typename T>
	typename T::Mask SMVector3<T>::operator==(const SMVector3<T>& aOther) const
	{
		return x.Compare(aOther.x, simdshared::CompareFunctions::Equal) & 
			y.Compare(aOther.y, simdshared::CompareFunctions::Equal) & 
			z.Compare(aOther.z, simdshared::CompareFunctions::Equal);
	}

	template <typename T>
	T SMVector3<T>::LengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	template <typename T>
	T SMVector3<T>::Length() const
	{
		return LengthSqr().Sqrt();
	}

	template <typename T>
	T SMVector3<T>::InvLength() const
	{
		return LengthSqr().InvSqrt();
	}

	template <typename T>
	void SMVector3<T>::SetLength(T aLength)
	{
		T ratio = aLength * InvLength();
		x *= ratio;
		y *= ratio;
		z *= ratio;
	}

	template <typename T>
	void SMVector3<T>::Normalize()
	{
		SetLength(1.0f);
	}

	template <typename T>
	T SMVector3<T>::Dot(const SMVector3& aVector) const
	{
		return x.MultiplyAdd(aVector.x, y.MultiplyAdd(aVector.y, z * aVector.z));
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::Cross(const SMVector3& aVector) const
	{
		return {
			(y * aVector.z) - (z * aVector.y),
			(z * aVector.x) - (x * aVector.z),
			(x * aVector.y) - (y * aVector.x)
		};
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::ddx() const
	{
		return {
			x.ddx(), y.ddx(), z.ddx()
		};
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::ddy() const
	{
		return {
			x.ddy(), y.ddy(), z.ddy()
		};
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::Abs() const
	{
		return { x.Abs(), y.Abs(), z.Abs() };
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::GetNormalized() const
	{
		T invLength = InvLength();
		return { x * invLength, y * invLength, z * invLength };
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::Min(const SMVector3& aOther)
	{
		return { x.Min(aOther.x), y.Min(aOther.y), z.Min(aOther.z) };
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::Max(const SMVector3& aOther)
	{
		return { x.Max(aOther.x), y.Max(aOther.y), z.Max(aOther.z) };
	}

	template <typename T>
	SMVector3<T> SMVector3<T>::ifelse(const typename T::Mask& aMask, const SMVector3& aTrue, const SMVector3& aFalse)
	{
		SMVector3 result{};
		result[0] = T::ifelse(aMask, aTrue[0], aFalse[0]);
		result[1] = T::ifelse(aMask, aTrue[1], aFalse[1]);
		result[2] = T::ifelse(aMask, aTrue[2], aFalse[2]);
		return result;
	}


	using SMVector3_4 = SMVector3<simdsse::SMFloat>;
	using SMVector3_8 = SMVector3<simdavx::SMFloat>;
	using SMVector3_16 = SMVector3<simdavx512::SMFloat>; // BARELY ANY PC SUPPORTS THIS EXTENSION!!!
}

template <typename T>
__forceinline SAE::SMVector3<T> operator*(float lhs, const SAE::SMVector3<T>& rhs)
{
	return rhs * lhs;
}

template <typename T>
__forceinline SAE::SMVector3<T> operator*(T lhs, const SAE::SMVector3<T>& rhs)
{
	return rhs * lhs;
}

template <typename T>
__forceinline SAE::SMVector3<T> operator/(float lhs, const SAE::SMVector3<T>& rhs)
{
	return SAE::SMVector3<T>(lhs) / rhs;
}

template <typename T>
__forceinline SAE::SMVector3<T> operator/(T lhs, const SAE::SMVector3<T>& rhs)
{
	return SAE::SMVector3<T>(lhs) / rhs;
}