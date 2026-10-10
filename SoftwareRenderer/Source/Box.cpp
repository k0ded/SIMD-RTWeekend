#include "Box.h"

#include <algorithm>

Box::Box(float x, float y, float z)
{
	min[0] = max[0] = x;
	min[1] = max[1] = y;
	min[2] = max[2] = z;
}

void Box::GetCenter(float& outX, float& outY, float& outZ) const
{
	outX = max[0] - min[0];
	outY = max[1] - min[1];
	outZ = max[2] - min[2];
}

const float* Box::GetMin() const
{
	return min;
}

const float* Box::GetMax() const
{
	return max;
}

void Box::AddPoint(float aX, float aY, float aZ)
{
	min[0] = std::min(min[0], aX);
	max[0] = std::max(max[0], aX);

	min[1] = std::min(min[1], aY);
	max[1] = std::max(max[1], aY);

	min[2] = std::min(min[2], aZ);
	max[2] = std::max(max[2], aZ);
}

void Box::AddBox(const Box& aOther)
{
	min[0] = std::min(min[0], aOther.min[0]);
	max[0] = std::max(max[0], aOther.max[0]);

	min[1] = std::min(min[1], aOther.min[1]);
	max[1] = std::max(max[1], aOther.max[1]);

	min[2] = std::min(min[2], aOther.min[2]);
	max[2] = std::max(max[2], aOther.max[2]);
}

bool Box::Intersects(const Box& aOther) const
{
	bool intersects = true;
	for (int i = 0; i < 3; ++i)
	{
		intersects = intersects && min[i] <= aOther.max[i] && max[i] >= aOther.min[i];
	}
	return intersects;
}

Box Box::SplitMax(int direction) const
{
	Box result = *this;
	result.min[direction] += (max[direction] - min[direction]) * 0.5f;
	return result;
}

Box Box::SplitMin(int direction) const
{
	Box result = *this;
	result.max[direction] -= (max[direction] - min[direction]) * 0.5f;
	return result;
}
