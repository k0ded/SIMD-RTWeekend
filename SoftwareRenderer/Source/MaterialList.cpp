#include "MaterialList.h"

#include "Material.h"

void MaterialList::RegisterMaterial(const std::shared_ptr<Material>& aMaterial)
{
	aMaterial->materialId = static_cast<int>(materials.size());
	materials.push_back(aMaterial);
}

std::shared_ptr<Material> MaterialList::GetMaterial(int aMaterialId) const
{
	return materials[aMaterialId];
}
