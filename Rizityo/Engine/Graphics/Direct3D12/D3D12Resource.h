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
		DescriptorHeap* _Container{ nullptr };
		uint32 _Index = UINT32_INVALID_NUM;
#endif // _DEBUG

	};

	class DescriptorHeap
	{
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : _Type{ type } {}
		DISABLE_COPY_AND_MOVE(DescriptorHeap);
		~DescriptorHeap() { assert(!_Heap); }

		bool Initialize(uint32 capacity, bool isShaderVisible);
		void ProcessDeferredFree(uint32 frameIndex);
		void Release();

		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE Type() const { return _Type; }
		constexpr ID3D12DescriptorHeap* const Heap() const { return _Heap; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE CPUStart() const { return _CPUStart; }
		constexpr D3D12_GPU_DESCRIPTOR_HANDLE GPUStart() const { return _GPUStart; }
		constexpr uint32 Capacity() const { return _Capacity; }
		constexpr uint32 Size() const { return _Size; }
		constexpr uint32 DescriptorSize() const { return _DescriptorSize; }
		constexpr bool IsShaderVisible() const { return _GPUStart.ptr != 0; }

	private:
		const D3D12_DESCRIPTOR_HEAP_TYPE _Type;
		ID3D12DescriptorHeap* _Heap;
		D3D12_CPU_DESCRIPTOR_HANDLE _CPUStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE _GPUStart{};
		std::unique_ptr<uint32[]> _FreeHandles{};
		Utility::Vector<uint32> _DeferredFreeIndices[FrameBufferCount]{};
		uint32 _Capacity = 0;
		uint32 _Size = 0;
		uint32 _DescriptorSize = 0;
		std::mutex _Mutex{};
	};

	struct D3D12TextureInitInfo
	{
		ID3D12Heap1* Heap = nullptr;
		ID3D12Resource* Resource = nullptr;
		D3D12_SHADER_RESOURCE_VIEW_DESC* SRVDesc = nullptr;
		D3D12_RESOURCE_DESC* Desc = nullptr;
		D3D12_RESOURCE_ALLOCATION_INFO1 AllocationInfo{};
		D3D12_RESOURCE_STATES InitialState{};
		D3D12_CLEAR_VALUE ClearValue{};
	};

	class D3D12Texture
	{
	public:
		D3D12Texture() = default;

		explicit D3D12Texture(D3D12TextureInitInfo info);

		~D3D12Texture()
		{
			Release();
		}

		DISABLE_COPY(D3D12Texture);

		constexpr D3D12Texture(D3D12Texture&& other) : _Resource{ other._Resource }, _SRV{ other._SRV }
		{
			other.Reset();
		}

		constexpr D3D12Texture& operator=(D3D12Texture&& other)
		{
			assert(this != &other);
			if (this != &other)
			{
				Release();
				Move(other);
			}
			return *this;
		}

		constexpr static uint32 MaxMips = 14;

		void Release();

		constexpr ID3D12Resource* const Resource() const { return _Resource; }
		constexpr DescriptorHandle SRV() const { return _SRV; }

	private:
		ID3D12Resource* _Resource = nullptr;
		DescriptorHandle _SRV;

		constexpr void Reset()
		{
			_Resource = nullptr;
			_SRV = {};
		}

		constexpr void Move(D3D12Texture& other)
		{
			_Resource = other._Resource;
			_SRV = other._SRV;
			other.Reset();
		}
	};

	class D3D12RenderTexture
	{
	public:
		D3D12RenderTexture() = default;

		explicit D3D12RenderTexture(D3D12TextureInitInfo info);

		~D3D12RenderTexture()
		{
			Release();
		}

		DISABLE_COPY(D3D12RenderTexture);

		constexpr D3D12RenderTexture(D3D12RenderTexture&& other)
			: _Texture{ std::move(other._Texture) }, _MipCount{ other._MipCount }
		{
			for (uint32 i = 0; i < _MipCount; i++)
			{
				_RTV[i] = other._RTV[i];
			}
			other.Reset();
		}

		constexpr D3D12RenderTexture& operator=(D3D12RenderTexture&& other)
		{
			assert(this != &other);
			if (this != &other)
			{
				Release();
				Move(other);
			}
			return *this;
		}

		void Release();

		constexpr uint32 MipCount() const { return _MipCount; }
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV(uint32 mipIndex) const
		{
			assert(mipIndex < _MipCount);
			return _RTV[mipIndex].CPU;
		}
		constexpr DescriptorHandle SRV() const { return _Texture.SRV(); }
		constexpr ID3D12Resource* const Resource() const { return _Texture.Resource(); }

	private:
		D3D12Texture _Texture{};
		DescriptorHandle _RTV[D3D12Texture::MaxMips]{};
		uint32 _MipCount = 0;

		constexpr void Reset()
		{
			for (uint32 i = 0; i < _MipCount; i++)
			{
				_RTV[i] = {};
			}
			_MipCount = 0;
		}

		constexpr void Move(D3D12RenderTexture& other)
		{
			_Texture = std::move(other._Texture);
			_MipCount = other._MipCount;
			for (uint32 i = 0; i < _MipCount; i++)
			{
				_RTV[i] = other._RTV[i];
			}
			other.Reset();
		}

	};

	class D3D12DepthBuffer
	{
		D3D12DepthBuffer() = default;

		explicit D3D12DepthBuffer(D3D12TextureInitInfo info);
		
		DISABLE_COPY(D3D12DepthBuffer);

		constexpr D3D12DepthBuffer(D3D12DepthBuffer&& other)
			: _Texture{ std::move(other._Texture) }, _DSV{ other._DSV }
		{
			other._DSV = {};
		}

		constexpr D3D12DepthBuffer& operator=(D3D12DepthBuffer&& other)
		{
			assert(this != &other);
			if (this != &other)
			{
				_Texture = std::move(other._Texture);
				_DSV = other._DSV;
				other._DSV = {};
			}
			return *this;
		}

		void Release();

		constexpr D3D12_CPU_DESCRIPTOR_HANDLE DSV() const { return _DSV.CPU; }
		constexpr DescriptorHandle SRV() const { return _Texture.SRV(); }
		constexpr ID3D12Resource* const Resource() const { return _Texture.Resource(); }

	private:
		D3D12Texture _Texture{};
		DescriptorHandle _DSV{};
	};
}