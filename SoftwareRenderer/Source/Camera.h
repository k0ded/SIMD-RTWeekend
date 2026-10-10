#pragma once

#include "Hittable.h"
#include "MaterialList.h"
#include "Texture.h"

class Camera
{
public:
    double aspect_ratio         = 1.0;  // Ratio of image width over height
    int    image_width          = 100;  // Rendered image width in pixel count
    int    samples_per_pixel    = 10;   // Count of random samples for each pixel
    int    max_depth            = 10;   // Maximum number of ray bounces into scene

    double vfov         = 90;           // Vertical view angle (field of view)
    Vec3  lookfrom  = {0.0f, 0.0f, 0.0f};    // Point camera is looking from
    Vec3  lookat    = {0.0f, 0.0f, -1.0f};   // Point camera is looking at
    Vec3  vup       = {0.0f, 1.0f, 0.0f};    // Camera-relative "up" direction

    float defocus_angle = 0;  // Variation angle of rays through each pixel
    float focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render(const Hittable& world, const MaterialList& materials);

private:
    constexpr static bool enableGradientFuncs = true;

    Texture backBuffer {};
    int image_height;   // Rendered image height
    float pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    Vec3 center;         // Camera center
    Vec3 pixel00_loc;    // Location of pixel 0, 0
    Vec3 pixel_delta_u;  // Offset to pixel to the right
    Vec3 pixel_delta_v;  // Offset to pixel below
    Vec3 u, v, w;      // Camera frame basis vectors
    Vec3 defocus_disk_u; // Defocus disk horizontal radius
    Vec3 defocus_disk_v; // Defocus disk vertical radius

    void Initialize();
    Ray GetRay(Float pixX, Float pixY) const;
    Vec3 sample_square() const;
    Vec3 defocus_disc_sample() const;

    std::atomic_int raysEvaluated = 0;
    Vec3 RayColor(const Ray& ray, int depth, const Hittable& world, const MaterialList& materials);
};
