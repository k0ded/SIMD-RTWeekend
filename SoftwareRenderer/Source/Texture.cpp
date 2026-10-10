#include "Texture.h"
#include "morton.h"

unsigned NextPo2(unsigned aInput)
{
    unsigned int v = aInput; // compute the next highest power of 2 of 32-bit v

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void Texture::Initialize(unsigned aWidth, unsigned aHeight, ETextureSwizzling aSwizzling)
{
    unsigned short xDiv, yDiv;
    morton_decode_2d(Float::ChannelCount() - 1, xDiv, yDiv);

	unsigned xSize = NextPo2(aWidth) / (xDiv + 1);
	unsigned ySize = NextPo2(aHeight) / (yDiv + 1);
    textureSizeX = aWidth;
    textureSizeY = aHeight;
	textureSwizzling = aSwizzling;
	myColors = new Vec3[xSize * ySize];
}

void Texture::Destroy()
{
	delete[] myColors;
}

void Texture::WriteTextureRGBA(unsigned aX, unsigned aY, float color[4])
{
    unsigned i = ToDataIndex(aX, aY, 0, textureSwizzling);
    unsigned simdVarIndex = i / 4;
    unsigned simdChannelIndex = i % 4;

    myColors[simdVarIndex][simdChannelIndex][0] = color[0];
    myColors[simdVarIndex][simdChannelIndex][1] = color[1];
    myColors[simdVarIndex][simdChannelIndex][2] = color[2];
    //myColors[simdVarIndex][simdChannelIndex][3] = color[3];
}

void Texture::WriteTexture(unsigned aBaseX, unsigned aBaseY, const Vec3& aColor) const
{
    unsigned i = ToDataIndex(aBaseX, aBaseY, 0, textureSwizzling);
    unsigned simdVarIndex = i / Vec3::ChannelCount();
    assert(i % Vec3::ChannelCount() == 0);
    myColors[simdVarIndex] = aColor;
}

void Texture::ExportTexture(uint32_t* aOutput, ETextureSwizzling aOutputSwizzling) const
{
	for (int y = 0; y < textureSizeY; ++y)
	{
		for (int x = 0; x < textureSizeX; ++x)
		{
            unsigned i = ToDataIndex(x, y, 0, textureSwizzling);
            unsigned simdVarIndex = i / Vec3::ChannelCount();
            unsigned simdChannelIndex = i % Vec3::ChannelCount();

            unsigned outputIndex = ToDataIndex(x, y, 0, aOutputSwizzling);
            uint32_t r = uint32_t(myColors[simdVarIndex][0][simdChannelIndex] * 255.99999f) & 0xFF;
            uint32_t g = uint32_t(myColors[simdVarIndex][1][simdChannelIndex] * 255.99999f) & 0xFF;
            uint32_t b = uint32_t(myColors[simdVarIndex][2][simdChannelIndex] * 255.99999f) & 0xFF;
            //uint32_t a = uint32_t(myColors[simdVarIndex][simdChannelIndex][3] * 255.99999f) & 0xFF;

            aOutput[outputIndex] = b << 16 | g << 8 | r;
		}
	}
}

unsigned Texture::TotalTextureSize() const
{
    return textureSizeX * textureSizeY;
}

unsigned Texture::ToDataIndex(uint16_t aX, uint16_t aY, uint16_t aChannel, ETextureSwizzling aSwizzle) const
{
    switch (aSwizzle) {
    case ETextureSwizzling::RGBAShaderAccess:
        return morton_encode_2d(aX, aY);
    case ETextureSwizzling::RGBALinear:
        return aY * textureSizeX + aX;
    }

    return -1;
}

void Texture::FromDataIndex(unsigned aDataIndex, ETextureSwizzling aSwizzle, uint16_t& outX, uint16_t& outY) const
{
    switch (aSwizzle) {
    case ETextureSwizzling::RGBAShaderAccess:
        morton_decode_2d(aDataIndex, outX, outY);
        return;
    case ETextureSwizzling::RGBALinear:
        outY = aDataIndex / textureSizeX;
        outX = aDataIndex % textureSizeY;
        return;
    }
}

void Texture::WriteMassDataRGBA(float* aPtr, unsigned aValueCount, ETextureSwizzling aInputSwizzle)
{
    for (unsigned i = 0; i < aValueCount; ++i)
    {
        uint16_t x, y;
        FromDataIndex(i, aInputSwizzle, x, y);
        WriteTextureRGBA(x, y, aPtr + (static_cast<size_t>(i) * 4ull));
    }
}
