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

	void Update(float32 dt)
	{
		for (const auto& ui : UIs)
		{
			ui->Update(dt);
		}
	}

	UI_Base::UI_Base(const char* name, uint32 width, uint32 height)
		: _GUI_Name{ name }, _Width{ width }, _Height{height}
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

		ImGui::SetNextWindowSize(ImVec2(_Width, _Height), ImGuiCond_Once);
		ImGui::Begin(_GUI_Name, &_ShowFlag);
		ShowContent();
		ImGui::End();
	}
}