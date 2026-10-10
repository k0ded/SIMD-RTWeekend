#pragma once
#include "Box.h"
#include "Interval.h"
#include "Ray.h"
#include "SIMDUsings.h"
#include "SIMD/SMVector3.h"

class Material;
class HitRecord {
public:
    Vec3 p;
    Vec3 normal;
    Float mat;
    Float t;
    Mask front_face;
    int totalObjectsEvaluated = 0;

    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = r.direction().Dot(outward_normal) <= 0.0f;
        normal = Vec3::ifelse(front_face, outward_normal, -outward_normal);
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;

    virtual Mask Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const = 0;

    const Box& GetBox() const
    {
        return boundingBox;
    }

protected:
    Box boundingBox { 0, 0, 0 };
};