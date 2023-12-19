#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::GUI
{
	bool Initialize();
	void Shutdown();

	void Show();
	void Render(ID3D12GraphicsCommandList* cmdList);
}