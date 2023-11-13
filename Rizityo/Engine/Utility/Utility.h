#pragma once


#define USE_STL_VECTOR 0
#define USE_STL_DEQUE 1

#if USE_STL_VECTOR
#include <vector>
namespace Rizityo::Utility
{
	template<typename T>
	using Vector = std::vector<T>;

	// íœ‚·‚é—v‘f‚ğ––”ö‚Ì—v‘f‚ÆŒğŠ·‚µ‚Ä‹ó‚«‚ª‚È‚¢‚æ‚¤‚Éíœ
	template<typename T>
	void EraseUnordered(std::vector<T>& v, size_t index)
	{
		if (v.size() > 1)
		{
			std::iter_swap(v.begin() + index, v.end() - 1);
			v.pop_back();
		}
		else
		{
			v.clear();
		}
	}
}
#else
#include "Container/Vector.h"
namespace Rizityo::Utility
{
	template<typename T>
	void EraseUnordered(Vector<T>& v, size_t index)
	{
		v.erase_unordered(index);
	}
}
#endif // USE_STL_VECTOR

#if USE_STL_DEQUE
#include <deque>
namespace Rizityo::Utility
{
	template<typename T>
	using Deque = std::deque<T>;
}
#endif // USE_STL_DEQUE

#include "Container/FreeList.h"