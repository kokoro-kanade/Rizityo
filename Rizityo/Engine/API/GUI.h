#pragma once
#include "CommonHeaders.h"
#include "ImGui/imgui.h"

// ImGui‚ðŽg‚¤‘O’ñ
namespace Rizityo::GUI
{
	class UI_Base
	{
	public:

		UI_Base() = delete;
		explicit UI_Base(const char* name) : UI_Base(name, 500, 500) {}
		explicit UI_Base(const char* name, uint32 width, uint32 height);

		void Show();
		virtual void Update(float32 dt) = 0;
		void SetFlag(bool flag) { _ShowFlag = flag; }

		[[nodiscard]] constexpr bool ShowFlag() const { return _ShowFlag; }

	protected:

		virtual void ShowContent() = 0;
		~UI_Base();

		bool _ShowFlag = false;

	private:

		const char* _GUI_Name;
		uint32 _Width;
		uint32 _Height;
	};
}

