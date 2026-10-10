#include "HittableList.h"

#include "HittablePool.h"
#include "Hittable/BVHBox.h"

HittableList::HittableList(const shared_ptr<Hittable>& object)
{
	add(object);
}

void HittableList::add(const shared_ptr<Hittable>& object)
{
	objects.push_back(object);

	if (objects.size() == 1)
	{
		boundingBox = object->GetBox();
	}
	else
	{
		boundingBox.AddBox(object->GetBox());
	}
}

void HittableList::buildAS(int direction)
{
	if (objects.size() < 8 || direction > 128)
	{
		Initialize();
		return;
	}

	std::vector<std::shared_ptr<Hittable>> openList = objects;
	objects.clear();

	Box bounds = boundingBox;

	auto leftBox = FreeListAllocator<BVHBox>::GetInstance()->Alloc();
	auto rightBox = FreeListAllocator<BVHBox>::GetInstance()->Alloc();

	bool isMaxEqual;
	bool isMinEqual;

	do
	{
		leftBox->objects.clear();
		rightBox->objects.clear();
		Box leftBounds = bounds.SplitMin(direction % 3);
		Box rightBounds = bounds.SplitMax(direction % 3);

		for (const auto& i : openList)
		{
			auto box = i->GetBox();
			if (leftBounds.Intersects(box))
			{
				leftBox->add(i);
			}

			if (rightBounds.Intersects(box))
			{
				rightBox->add(i);
			}
		}
		isMaxEqual = abs(leftBox->GetBox().GetMax()[direction % 3] - rightBox->GetBox().GetMax()[direction % 3]) < 0.01f;
		isMinEqual = abs(leftBox->GetBox().GetMin()[direction % 3] - rightBox->GetBox().GetMin()[direction % 3]) < 0.01f;
		direction++;
	} while (isMaxEqual && isMinEqual);

	leftBox->buildAS(direction);
	rightBox->buildAS(direction);

	if (!leftBox->objects.empty())
		add(leftBox);

	if (!rightBox->objects.empty())
		add(rightBox);
	Initialize();
}

Mask HittableList::Hit(const Ray& r, const Interval& ray_t, HitRecord& rec) const
{
	HitRecord temp_rec;
	Mask hit_anything{};
	Float closest_so_far = ray_t.max;

	auto queryHit = [&](const std::shared_ptr<Hittable>& object)
		{
			auto mask = object->Hit(r, Interval(ray_t.min, closest_so_far), temp_rec);
			hit_anything |= mask;

			closest_so_far = Float::ifelse(mask, temp_rec.t, closest_so_far);
			rec.p = Vec3::ifelse(mask, temp_rec.p, rec.p);
			rec.normal = Vec3::ifelse(mask, temp_rec.normal, rec.normal);
			rec.t = Float::ifelse(mask, temp_rec.t, rec.t);
			rec.front_face = mask.ifelse(temp_rec.front_face, rec.front_face);
			rec.mat = Float::ifelse(mask, temp_rec.mat, rec.mat);
			rec.totalObjectsEvaluated += temp_rec.totalObjectsEvaluated;
		};

	for (const auto& object : objects) {
		queryHit(object);
	}

	return hit_anything;
}
