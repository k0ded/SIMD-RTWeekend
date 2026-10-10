#pragma once
#include <memory>

#include "../Hittable.h"

class Sphere : public Hittable
{
public:
	Sphere();
	Sphere(float aCenterX, float aCenterY, float aCenterZ, float aRadius, const std::shared_ptr<Material>& aMaterial);

	Mask Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const override;

private:
	Vec3 Center;
	Float InvRadius;
	Float Radius;
	Float Radius2;
	std::shared_ptr<Material> mat;
};
