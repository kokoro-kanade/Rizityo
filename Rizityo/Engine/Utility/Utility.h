#pragma once

#define USE_STL_VECTOR 1
#define USE_STL_DEQUE 1

#if USE_STL_VECTOR
#include <vector>
namespace Rizityo::Utility
{
	template<typename T>
	using Vector = std::vector<T>;

	// Rename: 名前から若干分かりづらい
	// 削除する要素を末尾の要素と交換して空きがないように削除
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
#endif // USE_STL_VECTOR

#if USE_STL_DEQUE
#include <deque>
namespace Rizityo::Utility
{
	template<typename T>
	using Deque = std::deque<T>;
}
#endif // USE_STL_DEQUE

