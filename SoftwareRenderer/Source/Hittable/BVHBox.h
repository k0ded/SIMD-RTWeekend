#pragma once
#include "../hittable.h"
#include "../HittableList.h"
#include "../SIMDUsings.h"

class BVHBox : public HittableList
{
public:
	BVHBox();
	void Initialize() override;

	Mask Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const override;

private:
	mutable bool initialized = false;
	mutable Vec3 Min;
	mutable Vec3 Max;
};
