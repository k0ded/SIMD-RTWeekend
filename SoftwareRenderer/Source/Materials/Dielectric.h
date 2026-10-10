#pragma once
#include "../Material.h"

class Dielectric : public Material
{
public:
	Dielectric(float aRefractionIndex) : refractionIndex(aRefractionIndex) {}

	Mask Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const override;

private:
	Float refractionIndex;

    static Float reflectance(Float cosine, Float refraction_index);
};
