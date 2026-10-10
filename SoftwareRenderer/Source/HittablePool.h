#pragma once
#include <memory>
#include <utility>
#include <vector>

template <typename T>
class FreeListAllocator
{
public:
	static FreeListAllocator** GetInstanceAddr()
	{
		static FreeListAllocator* s_instance;
		return &s_instance;
	}

	static FreeListAllocator* GetInstance()
	{
		return *GetInstanceAddr();
	}

	static void Initialize(unsigned aMaxObjectSize)
	{
		*GetInstanceAddr() = new FreeListAllocator(aMaxObjectSize);
	}

	template <class... Args>
	std::shared_ptr<T> Alloc(Args&&... args)
	{
		unsigned index = freeList.back();
		freeList.pop_back();
		T* result = Data + index;
		new(result) T(std::forward<Args>(args)...);
		return std::shared_ptr<T>(result, [this](T* aPtr)
			{
				Free(aPtr);
			}
		);
	}

	void Free(T* aPtr)
	{
		unsigned index = static_cast<unsigned>(aPtr - Data);
		freeList.push_back(index);
		aPtr->~T();
	}

private:
	FreeListAllocator(unsigned maxObjectCount)
	{
		Data = new T[maxObjectCount];

		freeList.resize(maxObjectCount);

		for (int i = 0; std::cmp_less(i, maxObjectCount); ++i)
		{
			freeList[i] = (maxObjectCount - 1) - i;
		}
	}

	T* Data;
	std::vector<unsigned> freeList;
};
