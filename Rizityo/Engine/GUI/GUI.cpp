#include "GUI.h"
#include "ImGui/imgui.h"

namespace Rizityo::GUI
{
	namespace
	{
		Vector<UI_Base*> UIs;
	}

	void Show()
	{
		for (const auto& ui : UIs)
		{
			ui->Show();
		}
	}

	UI_Base::UI_Base(const char* name) : _GUI_Name{name}
	{
		UIs.emplace_back(this);
	}

	UI_Base::~UI_Base()
	{
		for (uint32 i = 0; i < UIs.size(); i++)
		{
			if (UIs[i] == this)
			{
				EraseUnordered(UIs, i);
				break;
			}
		}
	}

	void UI_Base::Show()
	{
		if (!_ShowFlag)
			return;

		ImGui::Begin(_GUI_Name, &_ShowFlag);
		ShowContent();
		ImGui::End();
	}
}