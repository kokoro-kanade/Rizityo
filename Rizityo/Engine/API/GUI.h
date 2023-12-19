#pragma once
#include "CommonHeaders.h"
#include "ImGui/imgui.h"

// ImGui���g���O��
namespace Rizityo::GUI
{
	class UI_Base
	{
	public:

		UI_Base() = delete;
		explicit UI_Base(const char* name);

		void Show();
		void SetFlag(bool flag) { _ShowFlag = flag; }

		[[nodiscard]] constexpr bool ShowFlag() const { return _ShowFlag; }

	protected:

		virtual void ShowContent() = 0;

		~UI_Base();

	private:

		const char* _GUI_Name;
		bool _ShowFlag = true;
	};
}

