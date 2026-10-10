#include "Metal.h"

#include "../rtweekend.h"

Mask Metal::Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const
{
    Vec3 reflected = reflect(rayIn.direction(), record.normal);
    reflected.Normalize();
    reflected += fuzz * random_unit_vector();
    outScattered = Ray(record.p, reflected);
    attenuation = albedo;
    return outScattered.direction().Dot(record.normal) > 0.0f;
}
