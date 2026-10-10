#pragma once
#include "SIMDUsings.h"

class Ray
{
public:
	Ray() = default;
	Ray(const Vec3& aOrigin, const Vec3& aDirection) : orig(aOrigin), dir(aDirection)
	{
		invDir = 1.0f / aDirection;
		invLength = 1.0f / dir.Length();
		invLengthSqr = 1.0f / dir.LengthSqr();
	}

	const Vec3& origin() const { return orig; }
	const Vec3& direction() const { return dir; }
	const Vec3& invDirection() const { return invDir; }
	const Float& inverseLength() const { return invLength; }
	const Float& inverseLengthSqr() const { return invLengthSqr; }

	Vec3 at(Float t) const {
		return orig + t * dir;
	}

	static Ray ifelse(const Mask& aMask, const Ray& aTrue, const Ray& aFalse) 
	{
		Vec3 origin = Vec3::ifelse(aMask, aTrue.orig, aFalse.orig);
		Vec3 direction = Vec3::ifelse(aMask, aTrue.dir, aFalse.dir);
		return Ray(origin, direction);
	}

private:
	Vec3 orig;
	Vec3 dir;
	Vec3 invDir;
	Float invLength;
	Float invLengthSqr;
};
