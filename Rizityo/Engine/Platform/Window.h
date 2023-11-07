#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Platform
{
	DEFINE_ID_TYPE(WindowId);

	class Window
	{
	public:
		constexpr explicit Window(WindowId id) : Id{ id } {}
		constexpr Window() : Id{ Id::INVALID_ID } {}
		constexpr WindowId GetId() const { return Id; }
		constexpr bool IsValid() const { return Id::IsValid(Id); }

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
		
		WindowId Id{ Id::INVALID_ID };
	};
}