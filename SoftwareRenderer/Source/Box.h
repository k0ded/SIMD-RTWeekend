#pragma once

class Box
{
public:
	Box(float x, float y, float z);

	void GetCenter(float& outX, float& outY, float& outZ) const;
	const float* GetMin() const;
	const float* GetMax() const;

	void AddPoint(float aX, float aY, float aZ);
	void AddBox(const Box& aOther);

	bool Intersects(const Box& aOther) const;

	Box SplitMax(int direction) const;
	Box SplitMin(int direction) const;

private:
	float min[3];
	float max[3];
};
