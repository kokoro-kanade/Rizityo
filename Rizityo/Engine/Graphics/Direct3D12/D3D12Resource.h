#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12
{
	class DescriptorHeap;

	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE GPU{};

		constexpr bool IsValid() { return CPU.ptr != 0; }
		constexpr bool IsShaderVisible() { return GPU.ptr != 0; }

#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* Container{ nullptr };
		uint32 Index = UINT32_INVALID_NUM;
#endif // _DEBUG

	};

	class DescriptorHeap
	{
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : Type{ type } {}
		DISABLE_COPY_AND_MOVE(DescriptorHeap);
		~DescriptorHeap() { assert(!Heap); }

		bool Initialize(uint32 capacity, bool isShaderVisible);
		void ProcessDeferredFree(uint32 frameIndex);
		void Release();

		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return Type; }
		constexpr ID3D12DescriptorHeap* const GetHeap() const { return Heap; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStart() const { return CPUStart; }
		constexpr D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStart() const { return GPUStart; }
		constexpr uint32 GetCapacity() const { return Capacity; }
		constexpr uint32 GetSize() const { return Size; }
		constexpr uint32 GetDescriptorSize() const { return DescriptorSize; }
		constexpr bool IsShaderVisible() const { return GPUStart.ptr != 0; }

	private:
		const D3D12_DESCRIPTOR_HEAP_TYPE Type;
		ID3D12DescriptorHeap* Heap;
		D3D12_CPU_DESCRIPTOR_HANDLE CPUStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE GPUStart{};
		std::unique_ptr<uint32[]> FreeHandles{};
		Utility::Vector<uint32> DeferredFreeIndices[FrameBufferCount]{};
		uint32 Capacity = 0;
		uint32 Size = 0;
		uint32 DescriptorSize = 0;
		std::mutex Mutex{};
	};
}