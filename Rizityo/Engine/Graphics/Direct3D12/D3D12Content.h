#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Content
{
	namespace Submesh
	{

		ID::IDType Add(const uint8*& data);
		void Remove(ID::IDType ID);

	} // namespace submesh
}