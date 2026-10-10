#include "Sphere.h"

#include <memory>

#include "../Materials/Lambertian.h"

Sphere::Sphere() = default;
Sphere::Sphere(float aCenterX, float aCenterY, float aCenterZ, float aRadius, const std::shared_ptr<Material>& aMaterial)
{
	Center = { aCenterX, aCenterY, aCenterZ };
	InvRadius = 1.0f / aRadius;
	Radius = aRadius;
    Radius2 = aRadius * aRadius;
    mat = aMaterial;

    boundingBox = { aCenterX - aRadius, aCenterY - aRadius, aCenterZ - aRadius };
    boundingBox.AddPoint(aCenterX + aRadius, aCenterY + aRadius, aCenterZ + aRadius);
}

Mask Sphere::Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const
{
    rec.totalObjectsEvaluated++;
    Vec3 oc = Center - r.origin();
    auto a = r.direction().LengthSqr();
    auto h = r.direction().Dot(oc);
    auto c = oc.LengthSqr() - Radius2;

    auto discriminant = h * h - a * c;
    Mask activeMask = discriminant >= 0.0f;

    auto sqrtd = discriminant.Sqrt();

    // Find the nearest root that lies in the acceptable range.
    auto root1 = (h - sqrtd) * r.inverseLengthSqr();
    auto root2 = (h + sqrtd) * r.inverseLengthSqr();

    auto rootActiveMask = ray_t.surrounds(root1);
    auto root = Float::ifelse(rootActiveMask, root1, root2);
    rootActiveMask &= ray_t.surrounds(root2);
    activeMask &= rootActiveMask;

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outwardNormal = (rec.p - Center) * InvRadius;
    rec.set_face_normal(r, outwardNormal);
    rec.mat = static_cast<float>(mat->GetMaterialId()) + 0.5f;
    return activeMask;
}
