#pragma once
#include "../Material.h"

class Metal : public Material
{
public:
	Metal(const Vec3& aAlbedo, Float aFuzz) : albedo(aAlbedo), fuzz(aFuzz.Min(1.0f)) {}

	Mask Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const override;

private:
	Vec3 albedo;
	Float fuzz;
};
