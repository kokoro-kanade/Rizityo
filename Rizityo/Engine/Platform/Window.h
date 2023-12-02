#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Platform
{
	DEFINE_ID_TYPE(WindowID);

	class Window
	{
	public:

		constexpr explicit Window(WindowID id) : _ID{ id } {}
		constexpr Window() : _ID{ ID::INVALID_ID } {}
		constexpr WindowID ID() const { return _ID; }
		constexpr bool IsValid() const { return ID::IsValid(_ID); }

		void SetFullScreen(bool isFullScreen) const;
		bool IsFullScreen() const;
		void SetCaption(const wchar_t* caption) const;
		void* Handle() const;
		Math::DX_U32Vector4 Size() const;
		void Resize(uint32 width, uint32 height) const;
		uint32 Width() const;
		uint32 Height() const;
		bool IsClosed() const;

	private:
		
		WindowID _ID{ ID::INVALID_ID };
	};
}