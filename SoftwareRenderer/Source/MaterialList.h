#pragma once
#include <memory>
#include <vector>

class Material;

class MaterialList
{
public:

	template <typename T, typename... Args>
	std::shared_ptr<T> MakeMaterial(Args&&... args)
	{
		auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
		RegisterMaterial(ptr);
		return ptr;
	}

	void RegisterMaterial(const std::shared_ptr<Material>& aMaterial);
	std::shared_ptr<Material> GetMaterial(int aMaterialId) const;

private:
	std::vector<std::shared_ptr<Material>> materials{};
};
