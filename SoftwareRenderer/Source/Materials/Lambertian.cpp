#include "Lambertian.h"

#include "../rtweekend.h"

Mask Lambertian::Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const
{
    Vec3 scatter_direction = record.normal + random_unit_vector();

    Mask degenerateMask = scatter_direction.NearZero();
    scatter_direction = Vec3::ifelse(degenerateMask, record.normal, scatter_direction);

    outScattered = Ray(record.p, scatter_direction);
    attenuation = albedo;
    return ~Mask();
}
