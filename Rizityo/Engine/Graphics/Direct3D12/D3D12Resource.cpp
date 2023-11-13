#include "D3D12Resource.h"
#include "D3D12Core.h"

namespace Rizityo::Graphics::D3D12
{
	bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible)
	{
		std::lock_guard lock{ Mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		{
			isShaderVisible = false;
		}

		Release();

		ID3D12Device* const device{ Core::GetMainDevice() };
		assert(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = Type;
		desc.NodeMask = 0;

		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Heap)));
		if (FAILED(hr))
			return false;

		FreeHandles = std::move(std::make_unique<uint32[]>(capacity));
		Capacity = capacity;
		Size = 0;

		for (uint32 i = 0; i < capacity; i++)
		{
			FreeHandles[i] = i;
		}
		DEBUG_ONLY(for (uint32 i = 0; i < FrameBufferCount; i++) assert(DeferredFreeIndices[i].empty()));

		DescriptorSize = device->GetDescriptorHandleIncrementSize(Type);
		CPUStart = Heap->GetCPUDescriptorHandleForHeapStart();
		GPUStart = isShaderVisible ? Heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::ProcessDeferredFree(uint32 frameIndex)
	{
		std::lock_guard lock{ Mutex };
		assert(frameIndex < FrameBufferCount);

		Utility::Vector<uint32>& indices{ DeferredFreeIndices[frameIndex] };
		if (!indices.empty())
		{
			for (auto index : indices)
			{
				Size--;
				FreeHandles[Size] = index;
			}
			indices.clear();
		}
	}

	void DescriptorHeap::Release()
	{
		assert(!Size);
		Core::DeferredRelease(Heap);
	}

	DescriptorHandle DescriptorHeap::Allocate()
	{
		std::lock_guard lock{ Mutex };
		assert(Heap);
		assert(Size < Capacity);

		const uint32 index = FreeHandles[Size];
		const uint32 offset = index * DescriptorSize;
		Size++;

		DescriptorHandle handle;
		handle.CPU.ptr = CPUStart.ptr + offset;
		if (IsShaderVisible())
		{
			handle.GPU.ptr = GPUStart.ptr + offset;
		}

		DEBUG_ONLY(handle.Container = this);
		DEBUG_ONLY(handle.Index = index);

		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle& handle)
	{
		if (!handle.IsValid())
			return;

		std::lock_guard lock{ Mutex };
		assert(Heap && Size);
		assert(handle.Container == this);
		assert(handle.CPU.ptr >= CPUStart.ptr);
		assert((handle.CPU.ptr - CPUStart.ptr) % DescriptorSize == 0);
		assert(handle.Index < Capacity);
		const uint32 index = (uint32)(handle.CPU.ptr - CPUStart.ptr) / DescriptorSize;
		assert(handle.Index == index);

		const uint32 frameIndex = Core::GetCurrentFrameIndex();
		DeferredFreeIndices[frameIndex].push_back(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}
}