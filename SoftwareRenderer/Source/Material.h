#pragma once
#include "Hittable.h"

class Material
{
public:
	virtual ~Material() = default;

	virtual Mask Scatter(const Ray& rayIn, const HitRecord& record, Vec3& attenuation, Ray& outScattered) const
	{
		return {};
	}

	int GetMaterialId() const { return materialId; }

private:
	friend class MaterialList;

	int materialId;
};
