#include "rtweekend.h"

#include "Camera.h"
#include "Hittable.h"
#include "HittableList.h"
#include "HittablePool.h"
#include "MaterialList.h"
#include "Hittable/BVHBox.h"
#include "Hittable/Sphere.h"
#include "Materials/Dielectric.h"
#include "Materials/Lambertian.h"
#include "Materials/Metal.h"

int main()
{
    FreeListAllocator<Sphere>::Initialize(500);
    FreeListAllocator<BVHBox>::Initialize(1000);
    FreeListAllocator<HittableList>::Initialize(10);

    MaterialList materialList {};
    HittableList world {};
    auto glassMaterial = materialList.MakeMaterial<Dielectric>(1.5f);

    auto ground = materialList.MakeMaterial<Lambertian>(Vec3{ 0.5f, 0.5f, 0.5f });

    for (int a = -11; a < 11; ++a)
    {
	    for (int b = -11; b < 11; ++b)
	    {
            auto choose_mat = random_float_i();
            Vec3 center((float)a + 0.9f * random_float_i(), 0.2f, (float)b + 0.9f * random_float_i());

            if ((center - Vec3(4.0f, 0.2f, 0.0f)).Length()[0] > 0.9f) {
                shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedoX = random_float_i(0.0f, 1.0f) * random_float_i(0.0f, 1.0f);
                    auto albedoY = random_float_i(0.0f, 1.0f) * random_float_i(0.0f, 1.0f);
                    auto albedoZ = random_float_i(0.0f, 1.0f) * random_float_i(0.0f, 1.0f);
                    sphere_material = materialList.MakeMaterial<Lambertian>(Vec3{ albedoX, albedoY, albedoZ });
                    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(center[0][0], center[1][0], center[2][0], 0.2f, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedoX = random_float_i(0.5f, 1.0f);
                    auto albedoY = random_float_i(0.5f, 1.0f);
                    auto albedoZ = random_float_i(0.5f, 1.0f);
                    auto fuzz = random_float_i(0.0f, 0.5f);
                    sphere_material = materialList.MakeMaterial<Metal>(Vec3{albedoX, albedoY, albedoZ}, fuzz);
                    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(center[0][0], center[1][0], center[2][0], 0.2f, sphere_material));
                }
                else {
                    // glass
                    sphere_material = glassMaterial;
                    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(center[0][0], center[1][0], center[2][0], 0.2f, sphere_material));
                }
            }
	    }
    }

    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(0.0f, 1.0f, 0.0f, 1.0f, glassMaterial));

    auto material2 = materialList.MakeMaterial<Lambertian>(Vec3{ 0.4f, 0.2f, 0.1f });
    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(-4.0f, 1.0f, 0.0f, 1.0f, material2));

    auto material3 = materialList.MakeMaterial<Metal>(Vec3(0.7f, 0.6f, 0.5f), 0.0f);
    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(4.0f, 1.0f, 0.0f, 1.0f, material3));

    world.buildAS();
    world.add(FreeListAllocator<Sphere>::GetInstance()->Alloc(0.0f, -1000.0f, 0.0f, 1000.0f, ground));

    Camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = Vec3(13.0f, 2.0f, 3.0f);
    cam.lookat = Vec3(0.0f, 0.0f, 0.0f);
    cam.vup = Vec3(0.0f, 1.0f, 0.0f);

    cam.defocus_angle = 0.6f;
    cam.focus_dist = 10.0f;

    Random_Initialize();
    cam.render(world, materialList);
}
