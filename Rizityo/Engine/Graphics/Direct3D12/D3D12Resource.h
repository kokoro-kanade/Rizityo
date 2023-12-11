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

		uint32 Index = UINT32_INVALID_NUM;

#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* _Container{ nullptr };
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

		[[nodiscard]] constexpr D3D12_DESCRIPTOR_HEAP_TYPE Type() const { return _Type; }
		[[nodiscard]] constexpr ID3D12DescriptorHeap* const Heap() const { return _Heap; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE CPUStart() const { return _CPUStart; }
		[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GPUStart() const { return _GPUStart; }
		[[nodiscard]] constexpr uint32 Capacity() const { return _Capacity; }
		[[nodiscard]] constexpr uint32 Size() const { return _Size; }
		[[nodiscard]] constexpr uint32 DescriptorSize() const { return _DescriptorSize; }
		[[nodiscard]] constexpr bool IsShaderVisible() const { return _GPUStart.ptr != 0; }

	private:
		const D3D12_DESCRIPTOR_HEAP_TYPE _Type;
		ID3D12DescriptorHeap* _Heap;
		D3D12_CPU_DESCRIPTOR_HANDLE _CPUStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE _GPUStart{};
		std::unique_ptr<uint32[]> _FreeHandles{};
		Vector<uint32> _DeferredFreeIndices[FrameBufferCount]{};
		uint32 _Capacity = 0;
		uint32 _Size = 0;
		uint32 _DescriptorSize = 0;
		std::mutex _Mutex{};
	};

	struct D3D12BufferInitInfo
	{
		ID3D12Heap1* Heap = nullptr;
		const void* Data = nullptr;
		D3D12_RESOURCE_ALLOCATION_INFO1 AlocationInfo{};
		D3D12_RESOURCE_STATES InitialState{};
		D3D12_RESOURCE_FLAGS Flags{ D3D12_RESOURCE_FLAG_NONE };
		uint32 Size = 0;
		uint32 Stride = 0;
		uint32 ElementCount = 0;
		uint32 Alignment = 0;
		bool CreateUAV = false;
	};

	class D3D12Buffer
	{
	public:
		D3D12Buffer() = default;

		explicit D3D12Buffer(D3D12BufferInitInfo info, bool isCpuAccessible);

		DISABLE_COPY(D3D12Buffer);

		constexpr D3D12Buffer(D3D12Buffer&& o)
			: _Buffer{ o._Buffer }, _GPU_Address{ o._GPU_Address }, _Size{ o._Size }
		{
			o.Reset();
		}

		constexpr D3D12Buffer& operator=(D3D12Buffer&& o)
		{
			assert(this != &o);
			if (this != &o)
			{
				Release();
				Move(o);
			}

			return *this;
		}

		~D3D12Buffer()
		{ 
			Release(); 
		}

		void Release();
		[[nodiscard]] constexpr ID3D12Resource* const Buffer() const { return _Buffer; }
		[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GPU_Address() const { return _GPU_Address; }
		[[nodiscard]] constexpr uint32 Size() const { return _Size; }

	private:
		constexpr void Move(D3D12Buffer& o)
		{
			_Buffer = o._Buffer;
			_GPU_Address = o._GPU_Address;
			_Size = o._Size;
			o.Reset();
		}

		constexpr void Reset()
		{
			_Buffer = nullptr;
			_GPU_Address = 0;
			_Size = 0;
		}

		ID3D12Resource* _Buffer = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS _GPU_Address{ 0 };
		uint32 _Size = 0;
	};

	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;

		explicit ConstantBuffer(D3D12BufferInitInfo info);

		DISABLE_COPY_AND_MOVE(ConstantBuffer);

		~ConstantBuffer()
		{ 
			Release(); 
		}

		void Release()
		{
			_Buffer.Release();
			_CPU_Address = nullptr;
			_CPU_Offset = 0;
		}

		constexpr void Clear()
		{ 
			_CPU_Offset = 0; 
		}

		[[nodiscard]] uint8* const Allocate(uint32 size);

		template<typename T>
		[[nodiscard]] T* const Allocate()
		{
			return (T* const)Allocate(sizeof(T));
		}

		[[nodiscard]] constexpr ID3D12Resource* const Buffer() const { return _Buffer.Buffer(); }
		[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GPU_Address() const { return _Buffer.GPU_Address(); }
		[[nodiscard]] constexpr uint32 Size() const { return _Buffer.Size(); }
		[[nodiscard]] constexpr uint8* const CPU_Address() const { return _CPU_Address; }

		template<typename T>
		[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS ToGPU_Address(T* const allocation)
		{
			std::lock_guard lock{ _Mutex };
			assert(_CPU_Address);
			if (!_CPU_Address) return {};
			const uint8* const address = (const uint8* const)allocation;
			assert(address <= _CPU_Address + _CPU_Offset);
			assert(address >= _CPU_Address);
			const uint64 offset = (uint64)(address - _CPU_Address);
			return _Buffer.GPU_Address() + offset;
		}

		[[nodiscard]] constexpr static D3D12BufferInitInfo GetDefaultInitInfo(uint32 size)
		{
			assert(size);
			D3D12BufferInitInfo info{};
			info.Size = size;
			info.Alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
			return info;
		}

	private:
		D3D12Buffer _Buffer{};
		uint8* _CPU_Address = nullptr;
		uint32 _CPU_Offset = 0;
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

		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return _Resource; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return _SRV; }

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

		[[nodiscard]] constexpr uint32 MipCount() const { return _MipCount; }
		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV(uint32 mipIndex) const
		{
			assert(mipIndex < _MipCount);
			return _RTV[mipIndex].CPU;
		}
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return _Texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return _Texture.Resource(); }

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
	public:
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

		[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE DSV() const { return _DSV.CPU; }
		[[nodiscard]] constexpr DescriptorHandle SRV() const { return _Texture.SRV(); }
		[[nodiscard]] constexpr ID3D12Resource* const Resource() const { return _Texture.Resource(); }

	private:
		D3D12Texture _Texture{};
		DescriptorHandle _DSV{};
	};
}