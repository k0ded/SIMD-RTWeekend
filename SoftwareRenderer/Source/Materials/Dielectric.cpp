#include "Dielectric.h"

#include "../rtweekend.h"

Mask Dielectric::Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const
{
    attenuation = Vec3(1.0f, 1.0f, 1.0f);
    Float ri = Float::ifelse(record.front_face, 1.0f / refractionIndex, refractionIndex);

    Vec3 unit_direction = rayIn.direction().GetNormalized();
    Float cos_theta = (-unit_direction).Dot(record.normal).Min(1.0f);
    Float sin_theta = (1.0f - cos_theta * cos_theta).Sqrt();

    Mask cannot_refract = ri * sin_theta > 1.0f;
    Mask randomReflectance = reflectance(cos_theta, ri) > randomFloat();

    Vec3 refracted = Vec3::ifelse(cannot_refract | randomReflectance,
        reflect(unit_direction, record.normal),
        refract(unit_direction, record.normal, ri)
    );

    outScattered = Ray(record.p, refracted);
    return ~Mask();
}

Float Dielectric::reflectance(Float cosine, Float refraction_index)
{
	// Use Schlick's approximation for reflectance.
	Float r0 = (1.0f - refraction_index) / (1.0f + refraction_index);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * (1.0f - cosine).Pow(5.0f);
}
