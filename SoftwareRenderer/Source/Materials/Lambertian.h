#pragma once
#include "../Material.h"

class Lambertian : public Material
{
public:
	Lambertian(const Vec3& aAlbedo) : albedo(aAlbedo) {}

	Mask Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const override;

private:
	Vec3 albedo;
};
