#include "D3D12GUI.h"
#include "D3D12Core.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "GUI/GUI.h"

namespace Rizityo::Graphics::D3D12::GUI
{
	bool Initialize()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		return ImGui_ImplDX12_Init(Core::GetMainDevice(),
								   FrameBufferCount,
								   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
								   Core::GetSRVHeap().Heap(),
								   Core::GetSRVHeap().CPUStart(),
								   Core::GetSRVHeap().GPUStart());
	}

	void Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void Show()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		Rizityo::GUI::Show();
	}

	void Render(ID3D12GraphicsCommandList* cmdList)
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
	}
}