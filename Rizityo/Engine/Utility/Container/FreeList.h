#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Utility
{
	template<typename T>
	class FreeList
	{
		static_assert(sizeof(T) >= sizeof(uint32));

	public:
		FreeList() = default;

		explicit FreeList(uint32 capacity)
		{
			_Array.reserve(capacity);
		}

		~FreeList()
		{
			assert(!_Size);
		}

		// 要素を追加するインデックスを返す
		template<class... Args>
		constexpr uint32 Add(Args&&... args)
		{
			uint32 index = UINT32_INVALID_NUM;
			if (_NextFreeIndex == UINT32_INVALID_NUM)
			{
				index = (uint32)_Array.size();
				_Array.emplace_back(std::forward<Args>(args)...);
			}
			else
			{
				index = _NextFreeIndex;
				assert(index < _Array.size() && AlreadyRemoved(index));
				_NextFreeIndex = *reinterpret_cast<const uint32* const>(std::addressof(_Array[index])); // 上位4バイトに次のFreeIndexが書かれている
				new (std::addressof(_Array[index])) T(std::forward<Args>(args)...);
			}
			_Size++;
			return index;
		}

		constexpr void Remove(uint32 index)
		{
			assert(index < _Array.size() && !AlreadyRemoved(index));
			T& item{ _Array[index] };
			item.~T();
			DEBUG_ONLY(memset(std::addressof(_Array[index]), 0xcc, sizeof(T)));
			*reinterpret_cast<uint32* const>(std::addressof(_Array[index])) = _NextFreeIndex; // 上位4バイトに次のFreeIndexを書きこむ
			_NextFreeIndex = index;
			_Size--;
		}

		constexpr uint32 Size() const
		{
			return _Size;
		}

		constexpr uint32 Capacity() const
		{
			return _Array.size();
		}

		constexpr bool IsEmpty() const
		{
			return _Size == 0;
		}

		[[nodiscard]] constexpr T& operator[](uint32 index)
		{
			assert(index < _Array.size() && !AlreadyRemoved(index));
			return _Array[index];
		}

		[[nodiscard]] constexpr const T& operator[](uint32 index) const
		{
			assert(index < _Array.size() && !AlreadyRemoved(index));
			return _Array[index];
		}

	private:
		Utility::Vector<T, false> _Array;
		uint32 _Size = 0;
		uint32 _NextFreeIndex = UINT32_INVALID_NUM;

		constexpr bool AlreadyRemoved(uint32 index) const
		{
			// sizeof(T) == sizeof(uint32)のときはこのテストはできない
			if constexpr (sizeof(T) > sizeof(uint32))
			{
				uint32 i = (sizeof(uint32));
				const uint8* const p = reinterpret_cast<const uint8* const>(std::addressof(_Array[index]));
				while ((p[i] == 0xcc) && (i < sizeof(T)))
				{
					i++;
				}
				return i == sizeof(T);
			}
			else
			{
				return true;
			}
		}
	};
}