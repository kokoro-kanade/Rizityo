#pragma once
#include "CommonHeaders.h"

namespace Rizityo
{
	template<typename T, bool Destruct = true>
	class Vector
	{
	public:
		Vector() = default;

		constexpr Vector(uint64 size)
		{
			resize(size);
		}

		// すべての要素をvalueで初期化
		constexpr explicit Vector(uint64 size, const T& value)
		{
			resize(size, value);
		}

		~Vector()
		{
			Destroy();
		}

		constexpr Vector(const Vector& other)
		{
			*this = other;
		}

		constexpr Vector(Vector&& other) : _Capacity{ other._Capacity }, _Size{ other._Size }, _Data{ other._Data }
		{
			other.Reset();
		}

		constexpr Vector& operator=(const Vector& other)
		{
			// 型Tがアドレス演算子を書き換えているかもしれないのでaddressof()を使う
			assert(this != std::addressof(other));
			if (this != std::addressof(other))
			{
				clear();
				reserve(other._Size);
				for (auto& item : other)
				{
					emplace_back(item);
				}
				assert(_Size == other._Size);
			}

			return *this;
		}

		constexpr Vector& operator=(Vector&& other)
		{
			assert(this != std::addressof(other));
			if (this != std::addressof(other))
			{
				Destroy();
				Move(other);
			}

			return *this;
		}

		constexpr void push_back(const T& value)
		{
			emplace_back(value);
		}

		constexpr void push_back(T&& value)
		{
			emplace_back(std::move(value));
		}

		template<typename... Params>
		constexpr decltype(auto) emplace_back(Params&&... params)
		{
			if (_Size == _Capacity)
			{
				reserve(((_Capacity + 1) * 3) >> 1); // 約1.5倍
			}
			assert(_Size < _Capacity);

			T* const item = new (std::addressof(_Data[_Size])) T(std::forward<Params>(params)...);
			_Size++;
			return *item;
		}

		constexpr T* const erase(uint64 index)
		{
			assert(_Data && index < _Size);
			return erase(std::addressof(_Data[index]));
		}

		constexpr T* const erase(T* const item)
		{
			assert(_Data && item >= std::addressof(_Data[0]) && item < std::addressof(_Data[_Size]));
			if constexpr (Destruct)
			{
				item->~T();
			}
			_Size--;
			if (item < std::addressof(_Data[_Size]))
			{
				memcpy(item, item + 1, (std::addressof(_Data[_Size]) - item) * sizeof(T));
			}

			return item;
		}

		constexpr T* const erase_unordered(uint64 index)
		{
			assert(_Data && index < _Size);
			return erase_unordered(std::addressof(_Data[index]));
		}

		constexpr T* const erase_unordered(T* const item)
		{
			assert(_Data && item >= std::addressof(_Data[0]) && item < std::addressof(_Data[_Size]));
			if constexpr (Destruct)
			{
				item->~T();
			}
			_Size--;
			if (item < std::addressof(_Data[_Size]))
			{
				memcpy(item, std::addressof(_Data[_Size]), sizeof(T));
			}

			return item;
		}

		constexpr void clear()
		{
			if constexpr (Destruct)
			{
				DestructRange(0, _Size);
			}
			_Size = 0;
		}

		// リサイズして新しく追加した要素をデフォルト値で初期化
		constexpr void resize(uint64 newSize)
		{
			static_assert(std::is_default_constructible<T>::value, "型はデフォルトコンストラクタを持っていなければいけません");
			if (newSize > _Size)
			{
				reserve(newSize);
				while (_Size < newSize)
				{
					emplace_back();
				}
			}
			else if (newSize < _Size)
			{
				if constexpr (Destruct)
				{
					DestructRange(newSize, _Size);
				}

				_Size = newSize;
			}
			assert(newSize == _Size);
		}

		// リサイズして新しく追加した要素を引数で初期化
		constexpr void resize(uint64 newSize, const T& value)
		{
			static_assert(std::is_copy_constructible<T>::value, "型はコピーコンストラクタを持っていなければいけません");
			if (newSize > _Size)
			{
				reserve(newSize);
				while (_Size < newSize)
				{
					emplace_back(value);
				}
			}
			else if (newSize < _Size)
			{
				if constexpr (Destruct)
				{
					DestructRange(newSize, _Size);
				}

				_Size = newSize;
			}
			assert(newSize == _Size);
		}

		constexpr void reserve(uint64 newCapacity)
		{
			if (newCapacity > _Capacity)
			{
				// realloc()は新しいメモリ領域が確保された場合に自動で元のデータをコピーする
				void* newBuffer = realloc(_Data, newCapacity * sizeof(T));
				assert(newBuffer);
				if (newBuffer)
				{
					_Data = static_cast<T*>(newBuffer);
					_Capacity = newCapacity;
				}
			}
		}

		constexpr void swap(Vector& other)
		{
			if (this != std::addressof(other))
			{
				auto tmp(std::move(other));
				other.Move(*this);
				Move(tmp);
			}
		}

		[[nodiscard]] constexpr T* data()
		{
			return _Data;
		}

		[[nodiscard]] constexpr T* const data() const
		{
			return _Data;
		}

		[[nodiscard]] constexpr bool empty() const
		{
			return _Size == 0;
		}

		[[nodiscard]] constexpr uint64 size() const
		{
			return _Size;
		}

		[[nodiscard]] constexpr uint64 capacity() const
		{
			return _Capacity;
		}

		[[nodiscard]] constexpr T& operator[](uint64 index)
		{
			assert(_Data && index < _Size);
			return _Data[index];
		}

		[[nodiscard]] constexpr const T& operator[](uint64 index) const
		{
			assert(_Data && index < _Size);
			return _Data[index];
		}

		[[nodiscard]] constexpr T& front()
		{
			assert(_Data && _Size);
			return _Data[0];
		}

		[[nodiscard]] constexpr const T& front() const
		{
			assert(_Data && _Size);
			return _Data[0];
		}

		[[nodiscard]] constexpr T& back()
		{
			assert(_Data && _Size);
			return _Data[_Size - 1];
		}

		[[nodiscard]] constexpr const T& back() const
		{
			assert(_Data && _Size);
			return _Data[_Size - 1];
		}

		[[nodiscard]] constexpr T* begin()
		{
			return std::addressof(_Data[0]);
		}

		[[nodiscard]] constexpr const T* begin() const
		{
			return std::addressof(_Data[0]);
		}

		[[nodiscard]] constexpr T* end()
		{
			assert(!(_Data == nullptr && _Size > 0));
			return std::addressof(_Data[_Size]);
		}

		[[nodiscard]] constexpr const T* end() const
		{
			assert(!(_Data == nullptr && _Size > 0));
			return std::addressof(_Data[_Size]);
		}

	private:
		uint64 _Size = 0;
		uint64 _Capacity = 0;
		T* _Data = nullptr;

		constexpr void Reset()
		{
			_Capacity = 0;
			_Size = 0;
			_Data = nullptr;
		}

		constexpr void Move(Vector& other)
		{
			_Capacity = other._Capacity;
			_Size = other._Size;
			_Data = other._Data;
			other.Reset();
		}

		constexpr void Destroy()
		{
			assert([&] {return _Capacity ? _Data != nullptr : _Data == nullptr; }());
			clear();
			_Capacity = 0;
			if (_Data)
				free(_Data);
			_Data = nullptr;
		}

		constexpr void DestructRange(uint64 first, uint64 last)
		{
			assert(Destruct);
			assert(first <= _Size && last <= _Size && first <= last);
			if (_Data)
			{
				for (; first != last; first++)
				{
					_Data[first].~T();
				}
			}
		}

	};
}