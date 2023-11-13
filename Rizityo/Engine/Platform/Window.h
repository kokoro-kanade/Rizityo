#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Platform
{
	DEFINE_ID_TYPE(WindowID);

	class Window
	{
	public:
		constexpr explicit Window(WindowID id) : ID{ id } {}
		constexpr Window() : ID{ ID::INVALID_ID } {}
		constexpr WindowID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

		void SetFullScreen(bool isFullScreen) const;
		bool IsFullScreen() const;
		void* GetHandle() const;
		void SetCaption(const wchar_t* caption) const;
		Math::U32Vector4 GetSize() const;
		void Resize(uint32 width, uint32 height) const;
		uint32 GetWidth() const;
		uint32 GetHeight() const;
		bool IsClosed() const;

	private:
		
		WindowID ID{ ID::INVALID_ID };
	};
}