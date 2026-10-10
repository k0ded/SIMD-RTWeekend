#include "BVHBox.h"

#include "../SIMDUsings.h"

BVHBox::BVHBox() = default;

void BVHBox::Initialize()
{
	HittableList::Initialize();
    const float* minPtr = boundingBox.GetMin();
    const float* maxPtr = boundingBox.GetMax();
    Min = { minPtr[0], minPtr[1], minPtr[2] };
    Max = { maxPtr[0], maxPtr[1], maxPtr[2] };
    initialized = true;
}

Mask BVHBox::Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const
{
    auto invDir = r.invDirection();
    Float t1 = (Min[0] - r.origin()[0]) * invDir[0];
    Float t2 = (Max[0] - r.origin()[0]) * invDir[0];
    Float tmin_x = t1.Min(t2);
    Float tmax_x = t1.Max(t2);

    Float t3 = (Min[1] - r.origin()[1]) * invDir[1];
    Float t4 = (Max[1] - r.origin()[1]) * invDir[1];
    Float tmin_y = t3.Min(t4);
    Float tmax_y = t3.Max(t4);

    Float t5 = (Min[2] - r.origin()[2]) * invDir[2];
    Float t6 = (Max[2] - r.origin()[2]) * invDir[2];
    Float tmin_z = t5.Min(t6);
    Float tmax_z = t5.Max(t6);

    Float tmin = tmin_x.Max(tmin_y).Max(tmin_z);
    Float tmax = tmax_x.Min(tmax_y).Min(tmax_z);

    // If tmax < 0, ray is intersecting behind origin
    // If tmin > tmax, ray doesn't intersect
    Mask mask = tmax < ray_t.min | tmin > tmax | tmin > ray_t.max;

    rec.totalObjectsEvaluated++;
    if (mask.AllSet())
    {
        return {};
    }

	return HittableList::Hit(r, ray_t, rec);
}
