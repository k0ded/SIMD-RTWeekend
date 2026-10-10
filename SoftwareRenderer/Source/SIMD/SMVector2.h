#pragma once
#include <cassert>

#include "SMFloatAll.h"

namespace SAE
{
	template <typename T>
	class SMVector2
	{
	public:
		SMVector2() = default;
		SMVector2(const T& aXY);
		SMVector2(const T& aX, const T& aY) : x(aX), y(aY) {}

		// Copy constructor (compiler generated)
		SMVector2(const SMVector2& aVector) = default;

		// Assignment operator (compiler generated)
		SMVector2& operator=(const SMVector2& aVector3) = default;

		// Destructor (compiler generated)
		~SMVector2() = default;

		void Load(float aX, float aY, unsigned aChannelIndex)
		{
			x[aChannelIndex] = aX;
			y[aChannelIndex] = aY;
		}

		void Store(float* aReturn, unsigned aChannelIndex)
		{
			aReturn[0] = x[aChannelIndex];
			aReturn[1] = y[aChannelIndex];
		}

		T& operator[](size_t aIndex);

		// Returns a negated copy of the vector
		SMVector2 operator-() const;
		T::Mask operator==(const SMVector2& aOther) const;

		// Returns the squared length of the vector
		T LengthSqr() const;

		// Returns the length of the vector
		T Length() const;

		// Returns 1 / length of the vector
		T InvLength() const;

		// Sets the length of the vector to the provided distance
		void SetLength(T aLength);

		void Normalize();

		T Dot() const;
		T Cross(const SMVector2& aVector) const;

		SMVector2 Abs() const;

		[[nodiscard]] constexpr static unsigned long long ChannelCount()
		{
			return T::ChannelCount();
		}


		SMVector2& operator+=(const SMVector2& aOther)
		{
			x += aOther.x;
			y += aOther.y;
			return *this;
		}

		SMVector2 operator+(const SMVector2& aFactor) const
		{
			SMVector2 f = *this;
			f += aFactor;
			return f;
		}

		SMVector2& operator-=(const SMVector2& aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			return *this;
		}

		SMVector2 operator-(const SMVector2& aFactor) const
		{
			SMVector2 f = *this;
			f -= aFactor;
			return f;
		}

		SMVector2& operator*=(const SMVector2& aFactor)
		{
			x *= aFactor.x;
			y *= aFactor.y;
			return *this;
		}

		SMVector2 operator*(const SMVector2& aFactor) const
		{
			SMVector2 f = *this;
			f *= aFactor;
			return f;
		}

		SMVector2& operator*=(T aFactor)
		{
			x *= aFactor;
			y *= aFactor;
			return *this;
		}

		SMVector2 operator*(T aFactor) const
		{
			SMVector2 f = *this;
			f *= aFactor;
			return f;
		}

		SMVector2& operator/=(const SMVector2& aFactor)
		{
			x /= aFactor.x;
			y /= aFactor.y;
			return *this;
		}

		SMVector2 operator/(const SMVector2& aFactor) const
		{
			SMVector2 f = *this;
			f /= aFactor;
			return f;
		}

		SMVector2& operator/=(T aFactor)
		{
			x /= aFactor;
			y /= aFactor;
			return *this;
		}

		SMVector2 operator/(T aFactor) const
		{
			SMVector2 f = *this;
			f /= aFactor;
			return f;
		}

	private:
		T x, y;
	};

	template <typename T>
	SMVector2<T>::SMVector2(const T& aXYZ) : x(aXYZ), y(aXYZ)
	{
	}

	template <typename T>
	T& SMVector2<T>::operator[](size_t aIndex)
	{
		assert(aIndex < 3);
		return (&x)[aIndex];
	}

	template <typename T>
	SMVector2<T> SMVector2<T>::operator-() const
	{
		return { -x, -y };
	}

	template <typename T>
	typename T::Mask SMVector2<T>::operator==(const SMVector2<T>& aOther) const
	{
		return x.Compare(aOther.x, simdshared::CompareFunctions::Equal) &
			y.Compare(aOther.y, simdshared::CompareFunctions::Equal);
	}

	template <typename T>
	T SMVector2<T>::LengthSqr() const
	{
		return x * x + y * y;
	}

	template <typename T>
	T SMVector2<T>::Length() const
	{
		return LengthSqr().Sqrt();
	}

	template <typename T>
	T SMVector2<T>::InvLength() const
	{
		return LengthSqr().InvSqrt();
	}

	template <typename T>
	void SMVector2<T>::SetLength(T aLength)
	{
		T ratio = aLength * InvLength();
		x *= ratio;
		y *= ratio;
	}

	template <typename T>
	void SMVector2<T>::Normalize()
	{
		SetLength(1);
	}

	template <typename T>
	T SMVector2<T>::Dot() const
	{
		return y.MultiplyAdd(y, x * x); // Execution from right to left, trying to stay in cache for as long as possible.
	}

	template <typename T>
	T SMVector2<T>::Cross(const SMVector2& aVector) const
	{
		return {
			(x * aVector.y) - (y * aVector.x)
		};
	}

	template <typename T>
	SMVector2<T> SMVector2<T>::Abs() const
	{
		return { x.Abs(), y.Abs() };
	}


	using SMVector2_4 = SMVector2<simdsse::SMFloat>;
	using SMVector2_8 = SMVector2<simdavx::SMFloat>;
	using SMVector2_16 = SMVector2<simdavx512::SMFloat>; // BARELY ANY PC SUPPORTS THIS EXTENSION!!!
}