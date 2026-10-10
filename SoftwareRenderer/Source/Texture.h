#pragma once
#include "SIMDUsings.h"
#include "SIMD/SMVector3.h"

enum class ETextureSwizzling
{
	RGBAShaderAccess,
	RGBALinear,
};

class Texture
{
public:
	void Initialize(unsigned aWidth, unsigned aHeight, ETextureSwizzling aSwizzling);
	void Destroy();

	void WriteTextureRGBA(unsigned aX, unsigned aY, float color[4]);
	void WriteTexture(unsigned aBaseX, unsigned aBaseY, const Vec3& aColor) const;

	void ExportTexture(uint32_t* aOutput, ETextureSwizzling aOutputSwizzling = ETextureSwizzling::RGBALinear) const;
	unsigned TotalTextureSize() const;

	uint16_t textureSizeX;
	uint16_t textureSizeY;
	float uvToPixelCoord;

	ETextureSwizzling textureSwizzling;
	Vec3* myColors;

private:
	unsigned ToDataIndex(uint16_t aX, uint16_t aY, uint16_t aChannel, ETextureSwizzling aSwizzle) const;
	void FromDataIndex(unsigned aDataIndex, ETextureSwizzling aSwizzle, uint16_t& outX, uint16_t& outY) const;

	void WriteMassDataRGBA(float* aPtr, unsigned aValueCount, ETextureSwizzling aInputSwizzle);

public:
	template <size_t SIZE>
	void Initialize(unsigned aWidth, unsigned aHeight, ETextureSwizzling aTextureSwizzling, float(&array)[SIZE], ETextureSwizzling aInputSwizzling = ETextureSwizzling::RGBALinear)
	{
		static_assert(SIZE % 4 == 0);
		Initialize(aWidth, aHeight, aTextureSwizzling);
		WriteMassDataRGBA(array, SIZE / 4, aInputSwizzling);
	}
};
