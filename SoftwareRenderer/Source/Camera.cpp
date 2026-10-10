#include "Camera.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <utility>

#include "Material.h"
#include "morton.h"
#include "rtweekend.h"
#include "SIMDUsings.h"
#include "Texture.h"
#include "ThreadPool.h"
#include "SIMD/SMVector2.h"

void Camera::render(const Hittable& world, const MaterialList& materials)
{
	Initialize();

	int xStride = 1;
	int yStride = 1;

	// Render

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	Vec2 offset {};

	auto channelCount = Vec2::ChannelCount();

	for (int i = 0; std::cmp_less(i, channelCount); ++i)
	{
		unsigned short x, y;
		if constexpr (enableGradientFuncs)
		{
			morton_decode_2d(i, x, y);
		}
		else
		{
			x = static_cast<unsigned short>(i);
			y = 0;
		}

		offset[0][i] = x;
		offset[1][i] = y;
		xStride = std::max(x + 1, xStride);
		yStride = std::max(y + 1, yStride);
	}

	std::atomic_int threadsFinished = 0;
	auto job = [&](int baseX, int baseY, int targetX, int targetY)
		{
			for (int j = baseY; j < targetY; j += yStride) {
				for (int i = baseX; i < targetX; i += xStride) {
					Vec2 pixel{ static_cast<float>(i), static_cast<float>(j) };
					pixel += offset;

					Vec2 imageSize{};
					imageSize[0] = { static_cast<float>(image_width) };
					imageSize[1] = { static_cast<float>(image_height) };

					Vec3 color{ 0.0f };
					for (int sample = 0; sample < samples_per_pixel; ++sample)
					{
						auto ray = GetRay(pixel[0], pixel[1]);
						color += RayColor(ray, max_depth, world, materials);
					}

					auto result = color * pixel_samples_scale;
					// Gamma correct
					result[0] = result[0].Sqrt();
					result[1] = result[1].Sqrt();
					result[2] = result[2].Sqrt();
					backBuffer.WriteTexture(i, j, result);
				}
			}
		};

	auto start = std::chrono::high_resolution_clock::now();

	int gridSizeX = 64;
	int gridSizeY = 64;

	int threadsX = (image_width + gridSizeX - 1) / gridSizeX;
	int threadsY = (image_height + gridSizeY - 1) / gridSizeY;

	int xSliceSize = ((image_width + (xStride - 1)) / xStride) / threadsX;
	int ySliceSize = ((image_height + (yStride - 1)) / yStride) / threadsY;

	{
		ThreadPool pool { 12 };
		int threadsDispatched = 0;
		std::atomic_int threadsFinished = 0;
		for (int ySlice = 0; ySlice < threadsY; ++ySlice)
		{
			for (int xSlice = 0; xSlice < threadsX; ++xSlice)
			{
				pool.enqueue([&, xSlice, ySlice]
					{
						int targetX = xSliceSize * (xSlice + 1) * xStride;
						int targetY = ySliceSize * (ySlice + 1) * yStride;
						if (ySlice == (threadsY - 1))
						{
							targetY = image_height;
						}

						if (xSlice == (threadsX - 1))
						{
							targetX = image_width;
						}
						job(xSliceSize * xSlice * xStride, ySliceSize * ySlice * yStride, targetX, targetY);
						++threadsFinished;
					});
				threadsDispatched++;
			}
		}

		while (threadsDispatched != threadsFinished)
		{
			std::clog << "\rJOBS FINISHED: " << threadsFinished << "/" << threadsDispatched << ' ' << std::flush;
		}
	}

	uint32_t* pixels = new uint32_t[backBuffer.TotalTextureSize()];
	backBuffer.ExportTexture(pixels);

	auto ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

	for (int i = 0; i < image_width * image_height; ++i)
	{
		uint32_t unpackedR = pixels[i] & 0xFF;
		uint32_t unpackedG = (pixels[i] >> 8) & 0xFF;
		uint32_t unpackedB = (pixels[i] >> 16) & 0xFF;
		std::cout << unpackedR << ' ' << unpackedG << ' ' << unpackedB << '\n';
	}

	delete[] pixels;
	int raysCast = raysEvaluated;
	std::clog << "\rDone.                 \n" 
			  << "Finished rendering in: " << ms_duration.count() << "ms\n" 
			  << "Number of rays cast (SIMD/ITERATIONS): " << raysCast * Float::ChannelCount() << '/' << raysCast << std::flush;
}

void Camera::Initialize()
{
	image_height = int(image_width / aspect_ratio);
	image_height = (image_height < 1) ? 1 : image_height;

	pixel_samples_scale = 1.0f / static_cast<float>(samples_per_pixel);

	ETextureSwizzling backBufferSwizzling = ETextureSwizzling::RGBAShaderAccess;

	if constexpr (!enableGradientFuncs)
	{
		backBufferSwizzling = ETextureSwizzling::RGBALinear;
	}

	backBuffer = Texture();
	backBuffer.Initialize(image_width, image_height, backBufferSwizzling);

	center = lookfrom;

	// Determine viewport dimensions.
	//auto focal_length = (lookfrom - lookat).Length()[0];
	auto theta = degrees_to_radians(vfov);
	auto h = std::tanf(theta / 2.0f);
	//auto viewport_height = 2 * h * focal_length;
	auto viewport_height = 2 * h * focus_dist;
	float viewport_width = viewport_height * (float(image_width) / image_height);

	// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
	w = (lookfrom - lookat).GetNormalized();
	u = vup.Cross(w).GetNormalized();
	v = w.Cross(u);

	// Calculate the vectors across the horizontal and down the vertical viewport edges.
	auto viewport_u = viewport_width * u;
	auto viewport_v = viewport_height * -v;

	// Calculate the horizontal and vertical delta vectors from pixel to pixel.
	pixel_delta_u = viewport_u / static_cast<float>(image_width);
	pixel_delta_v = viewport_v / static_cast<float>(image_height);

	// Calculate the location of the upper left pixel.
	auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2.0f - viewport_v / 2.0f;
	pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

	// Calculate the camera defocus disk basis vectors.
	auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
	defocus_disk_u = u * defocus_radius;
	defocus_disk_v = v * defocus_radius;
}

Ray Camera::GetRay(Float pixX, Float pixY) const
{
	// Construct a camera ray originating from the defocus disk and directed at a randomly
	// sampled point around the pixel location i, j.

	auto offset = sample_square();
	auto pixel_sample = pixel00_loc
		+ ((offset[0] + pixX) * pixel_delta_u)
		+ ((offset[1] + pixY) * pixel_delta_v);

	auto ray_origin = defocus_angle <= 0.0f ? center : defocus_disc_sample();
	auto ray_direction = pixel_sample - ray_origin;

	return Ray(ray_origin, ray_direction);
}

Vec3 Camera::sample_square() const
{
	Float x = randomFloat() - 0.5f;
	Float y = randomFloat() - 0.5f;

	// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
	return Vec3(x, y, 0.0f);
}

Vec3 Camera::defocus_disc_sample() const
{
	// Returns a random point in the camera defocus disk.
	auto p = random_in_unit_disk();
	return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}

Vec3 Camera::RayColor(const Ray& ray, int depth, const Hittable& world, const MaterialList& materials)
{
	if (depth <= 0)
		return {};

	++raysEvaluated;

	HitRecord record;
	auto mask = world.Hit(ray, Interval(0.001f, infinity), record);
	Vec3 unit_direction = ray.direction().GetNormalized();
	auto a = 0.5f * (unit_direction[1] + 1.0f);
	auto miss = (1.0f - a) * Vec3(1.0f, 1.0f, 1.0f) + a * Vec3(0.5f, 0.7f, 1.0f);

	if (mask.CountHigh() == 0)
	{
		return miss;
	}

	Ray scattered;
	Vec3 attenuation;
	Mask didScatter {};

	if (record.mat.IsUniform())
	{
		// FAST :D
		auto materialId = (int)record.mat[0];
		didScatter = materials.GetMaterial(materialId)->Scatter(ray, record, attenuation, scattered);
	}
	else
	{
		int materialCount = 0;
		float materialIds[Float::ChannelCount()] = {};
		Ray tempScattered;
		Vec3 tempAttenuation;

		// Slow branch :(
		for (int i = 0; i < Float::ChannelCount(); ++i)
		{
			bool foundMaterial = false;
			float materialId = record.mat[i];
			for (int i = 0; i < materialCount; ++i)
			{
				if (materialIds[i] == materialId)
				{
					foundMaterial = true;
					break;
				}
			}

			if (!foundMaterial)
			{
				materialIds[materialCount] = materialId;
				materialCount++;
			}
		}

		for (int i = 0; i < materialCount; ++i)
		{
			auto selectedMaterialMask = record.mat == materialIds[i];

			auto materialResultMask = materials.GetMaterial(static_cast<int>(materialIds[i]))->Scatter(ray, record, tempAttenuation, tempScattered);
			auto materialScatteredMask = (materialResultMask & selectedMaterialMask);

			didScatter = didScatter | materialScatteredMask;
			attenuation = Vec3::ifelse(materialScatteredMask, tempAttenuation, attenuation);
			scattered = Ray::ifelse(materialScatteredMask, tempScattered, scattered);
		}
	}

	Vec3 hit = attenuation * RayColor(scattered, depth - 1, world, materials);

	hit = Vec3::ifelse(didScatter, hit, { 0.0f });
	auto result = Vec3::ifelse(mask, hit, miss);

	constexpr bool debugBVH = false;
	if (depth == max_depth && debugBVH)
	{
		float highFactor = std::max((float)record.totalObjectsEvaluated - 5.f, 0.0f) / 15.0f;
		float lowFactor = std::min((float)record.totalObjectsEvaluated, 10.0f) / 10.0f;
		result = Vec3({ highFactor, 0.0f, lowFactor });
	}
	return result;
}
