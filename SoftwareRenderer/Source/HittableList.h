#pragma once

#include "hittable.h"

#include <memory>
#include <vector>

#include "SIMDUsings.h"

using std::make_shared;
using std::shared_ptr;

class HittableList : public Hittable {
public:
    std::vector<shared_ptr<Hittable>> objects {};

    HittableList() {}
    HittableList(const shared_ptr<Hittable>& object);

    void clear() { objects.clear(); }

    void add(const shared_ptr<Hittable>& object);

    void buildAS(int direction = 0);

    virtual void Initialize() {}

	Mask Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const override;
};