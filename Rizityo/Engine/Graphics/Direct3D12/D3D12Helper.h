#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Helper
{
	constexpr struct 
	{
		const D3D12_HEAP_PROPERTIES DefaultHeap
		{
			D3D12_HEAP_TYPE_DEFAULT,						// Type
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,				// CPUPageProperty
			D3D12_MEMORY_POOL_UNKNOWN,						// MemoryPoolPreference
			0,												// CreationNodeMask
			0												// VisibleNodeMask
		};

		const D3D12_HEAP_PROPERTIES UploadHeap{
			D3D12_HEAP_TYPE_UPLOAD,                         // Type
			D3D12_CPU_PAGE_PROPERTY_UNKNOWN,                // CPUPageProperty
			D3D12_MEMORY_POOL_UNKNOWN,                      // MemoryPoolPreference
			0,                                              // CreationNodeMask
			0                                               // VisibleNodeMask
		};

	} HeapProperties ;

	constexpr struct
	{
		const D3D12_RASTERIZER_DESC NoCull
		{
			D3D12_FILL_MODE_SOLID,							// FillMode 
			D3D12_CULL_MODE_NONE,							// CullMode
			1,												// FrontCounterClockwise
			0,												// DepthBias
			0,												// DepthBiasClamp
			0,												// SlopeScaledDepthBias
			1,												// DepthClipEnable
			0,												// MultisampleEnable
			0,												// AntialiasedLineEnable
			0,												// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF		// ConservativeRaster
		};

		const D3D12_RASTERIZER_DESC BackfaceCull
		{
			D3D12_FILL_MODE_SOLID,							// FillMode 
			D3D12_CULL_MODE_BACK,							// CullMode
			1,												// FrontCounterClockwise
			0,												// DepthBias
			0,												// DepthBiasClamp
			0,												// SlopeScaledDepthBias
			1,												// DepthClipEnable
			0,												// MultisampleEnable
			0,												// AntialiasedLineEnable
			0,												// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF		// ConservativeRaster
		};

		const D3D12_RASTERIZER_DESC FrontfaceCull
		{
			D3D12_FILL_MODE_SOLID,							// FillMode 
			D3D12_CULL_MODE_FRONT,							// CullMode
			1,												// FrontCounterClockwise
			0,												// DepthBias
			0,												// DepthBiasClamp
			0,												// SlopeScaledDepthBias
			1,												// DepthClipEnable
			0,												// MultisampleEnable
			0,												// AntialiasedLineEnable
			0,												// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF		// ConservativeRaster
		};

		const D3D12_RASTERIZER_DESC WireFrame
		{
			D3D12_FILL_MODE_WIREFRAME,						// FillMode 
			D3D12_CULL_MODE_NONE,							// CullMode
			1,												// FrontCounterClockwise
			0,												// DepthBias
			0,												// DepthBiasClamp
			0,												// SlopeScaledDepthBias
			1,												// DepthClipEnable
			0,												// MultisampleEnable
			0,												// AntialiasedLineEnable
			0,												// ForcedSampleCount
			D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF		// ConservativeRaster
		};


	} RasterizerState ;

	constexpr struct
	{
		const D3D12_DEPTH_STENCIL_DESC1 Disabled
		{
			0,												// DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,					// DepthWriteMask
			D3D12_COMPARISON_FUNC_LESS_EQUAL,				// DepthFunc
			0,												// StencilEnable
			0,												// StencilReadMask
			0,												// StencilWriteMask
			{},												// FrontFace
			{},												// BackFace
			0												// DepthBoundsTestEnable
		};

		const D3D12_DEPTH_STENCIL_DESC1 Enabled
		{
			1,												// DepthEnable
			D3D12_DEPTH_WRITE_MASK_ALL,						// DepthWriteMask
			D3D12_COMPARISON_FUNC_LESS_EQUAL,				// DepthFunc
			0,												// StencilEnable
			0,												// StencilReadMask
			0,												// StencilWriteMask
			{},												// FrontFace
			{},												// BackFace
			0												// DepthBoundsTestEnable
		};

		const D3D12_DEPTH_STENCIL_DESC1 EnabledReadonly
		{
			1,                                              // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,                    // DepthWriteMask
			D3D12_COMPARISON_FUNC_LESS_EQUAL,               // DepthFunc
			0,                                              // StencilEnable
			0,                                              // StencilReadMask
			0,                                              // StencilWriteMask
			{},                                             // FrontFace
			{},                                             // BackFace
			0                                               // DepthBoundsTestEnable
		};

		const D3D12_DEPTH_STENCIL_DESC1 Reversed
		{
			1,                                              // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ALL,                     // DepthWriteMask
			D3D12_COMPARISON_FUNC_GREATER_EQUAL,            // DepthFunc
			0,                                              // StencilEnable
			0,                                              // StencilReadMask
			0,                                              // StencilWriteMask
			{},                                             // FrontFace
			{},                                             // BackFace
			0                                               // DepthBoundsTestEnable
		};

		const D3D12_DEPTH_STENCIL_DESC1 ReversedReadonly
		{
			1,                                              // DepthEnable
			D3D12_DEPTH_WRITE_MASK_ZERO,                    // DepthWriteMask
			D3D12_COMPARISON_FUNC_GREATER_EQUAL,            // DepthFunc
			0,                                              // StencilEnable
			0,                                              // StencilReadMask
			0,                                              // StencilWriteMask
			{},                                             // FrontFace
			{},                                             // BackFace
			0                                               // DepthBoundsTestEnable
		};

	} DepthState ;

	constexpr struct {
		const D3D12_BLEND_DESC Disabled
		{
			0,                                              // AlphaToCoverageEnable
			0,                                              // IndependentBlendEnable
			{
				{
				   0,                                       // BlendEnable
				   0,                                       // LogicOpEnable
				   D3D12_BLEND_SRC_ALPHA,                   // SrcBlend
				   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
				   D3D12_BLEND_OP_ADD,                      // BlendOp
				   D3D12_BLEND_ONE,                         // SrcBlendAlpha
				   D3D12_BLEND_ONE,                         // DestBlendAlpha
				   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
				   D3D12_LOGIC_OP_NOOP,                     // LogicOp
				   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

				},
				{},{},{},{},{},{},{}
			}
		};

		const D3D12_BLEND_DESC AlphaBlend
		{
			0,                                              // AlphaToCoverageEnable
			0,                                              // IndependentBlendEnable
			{
				{
				   1,                                       // BlendEnable
				   0,                                       // LogicOpEnable
				   D3D12_BLEND_SRC_ALPHA,                   // SrcBlend
				   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
				   D3D12_BLEND_OP_ADD,                      // BlendOp
				   D3D12_BLEND_ONE,                         // SrcBlendAlpha
				   D3D12_BLEND_ONE,                         // DestBlendAlpha
				   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
				   D3D12_LOGIC_OP_NOOP,                     // LogicOp
				   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

				},
				{},{},{},{},{},{},{}
			}
		};

		const D3D12_BLEND_DESC Additive
		{
			0,                                              // AlphaToCoverageEnable
			0,                                              // IndependentBlendEnable
			{
				{
				   1,                                       // BlendEnable
				   0,                                       // LogicOpEnable
				   D3D12_BLEND_ONE,                         // SrcBlend
				   D3D12_BLEND_ONE,                         // DestBlend
				   D3D12_BLEND_OP_ADD,                      // BlendOp
				   D3D12_BLEND_ONE,                         // SrcBlendAlpha
				   D3D12_BLEND_ONE,                         // DestBlendAlpha
				   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
				   D3D12_LOGIC_OP_NOOP,                     // LogicOp
				   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

				},
				{},{},{},{},{},{},{}
			}
		};

		const D3D12_BLEND_DESC Premultiplied
		{
			0,                                              // AlphaToCoverageEnable
			0,                                              // IndependentBlendEnable
			{
				{
				   0,                                       // BlendEnable
				   0,                                       // LogicOpEnable
				   D3D12_BLEND_ONE,                         // SrcBlend
				   D3D12_BLEND_INV_SRC_ALPHA,               // DestBlend
				   D3D12_BLEND_OP_ADD,                      // BlendOp
				   D3D12_BLEND_ONE,                         // SrcBlendAlpha
				   D3D12_BLEND_ONE,                         // DestBlendAlpha
				   D3D12_BLEND_OP_ADD,                      // BlendOpAlpha
				   D3D12_LOGIC_OP_NOOP,                     // LogicOp
				   D3D12_COLOR_WRITE_ENABLE_ALL,            // RenderTargetWriteMask

				},
				{},{},{},{},{},{},{}
			}
		};
	} BlendState;

	constexpr uint64 AlignSizeForConstantBuffer(uint64 size)
	{
		return Math::AlignSizeUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(size);
	}

	constexpr uint64 AlignSizeForTexture(uint64 size)
	{
		return Math::AlignSizeUp<D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT>(size);
	}

	class D3D12ResourceBarrier
	{
	public:
		constexpr static uint32 MaxResourceBarriers = 32;

		// Transitionバリアの追加
		constexpr void Add(ID3D12Resource* resource,
						   D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
						   D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
						   uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
		{
			assert(resource);
			assert(_NumBarriers < MaxResourceBarriers);

			D3D12_RESOURCE_BARRIER& barrier{ _Barriers[_NumBarriers] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = flags;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
			barrier.Transition.Subresource = subresource;

			_NumBarriers++;
		}

		// UAVバリアの追加
		constexpr void Add(ID3D12Resource* resource,
						   D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
		{
			assert(resource);
			assert(_NumBarriers < MaxResourceBarriers);

			D3D12_RESOURCE_BARRIER& barrier{ _Barriers[_NumBarriers] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.Flags = flags;
			barrier.UAV.pResource = resource;

			_NumBarriers++;
		}

		// Aliasingバリアの追加
		constexpr void Add(ID3D12Resource* resourceBefore, ID3D12Resource* resourceAfter,
						   D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
		{
			assert(resourceBefore && resourceAfter);
			assert(_NumBarriers < MaxResourceBarriers);

			D3D12_RESOURCE_BARRIER& barrier{ _Barriers[_NumBarriers] };
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Flags = flags;
			barrier.Aliasing.pResourceBefore = resourceBefore;
			barrier.Aliasing.pResourceAfter = resourceAfter;

			_NumBarriers++;
		}

		void Apply(ID3D12GraphicsCommandList* cmdList)
		{
			assert(_NumBarriers);
			cmdList->ResourceBarrier(_NumBarriers, _Barriers);
			_NumBarriers = 0;
		}

	private:
		D3D12_RESOURCE_BARRIER _Barriers[MaxResourceBarriers]{};
		uint32 _NumBarriers = 0;
	};

	void TransitionResource(ID3D12GraphicsCommandList* cmdList,
							ID3D12Resource* resource,
							D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
							D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
							uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	struct D3D12DescriptorRange : public D3D12_DESCRIPTOR_RANGE1
	{
		constexpr explicit D3D12DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
												uint32 descriptorCount, uint32 shaderRegister, uint32 space = 0,
												D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
												uint32 offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
			: D3D12_DESCRIPTOR_RANGE1{ rangeType, descriptorCount, shaderRegister, space, flags, offsetFromTableStart }
		{}
	};

	struct D3D12RootParameter : public D3D12_ROOT_PARAMETER1
	{
		constexpr void AsConstants(uint32 numConstants, D3D12_SHADER_VISIBILITY visibility,
								   uint32 shaderRegister, uint32 space = 0)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			ShaderVisibility = visibility;
			Constants.Num32BitValues = numConstants;
			Constants.ShaderRegister = shaderRegister;
			Constants.RegisterSpace = space;
		}

		constexpr void  AsCBV(D3D12_SHADER_VISIBILITY visibility,
							  uint32 shaderRegister, uint32 space = 0,
							  D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, visibility, shaderRegister, space, flags);
		}

		constexpr void  AsUAV(D3D12_SHADER_VISIBILITY visibility,
							  uint32 shaderRegister, uint32 space = 0,
							  D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, visibility, shaderRegister, space, flags);
		}

		constexpr void  AsSRV(D3D12_SHADER_VISIBILITY visibility,
							  uint32 shaderRegister, uint32 space = 0,
							  D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE)
		{
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, visibility, shaderRegister, space, flags);
		}

		constexpr void AsDescriptorTable(D3D12_SHADER_VISIBILITY visibility,
										 const D3D12DescriptorRange* ranges, uint32 rangeCount)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			ShaderVisibility = visibility;
			DescriptorTable.NumDescriptorRanges = rangeCount;
			DescriptorTable.pDescriptorRanges = ranges;
		}

	private:
		constexpr void AsDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility,
									uint32 shaderRegister, uint32 space, D3D12_ROOT_DESCRIPTOR_FLAGS flags)
		{
			ParameterType = type;
			ShaderVisibility = visibility;
			Descriptor.ShaderRegister = shaderRegister;
			Descriptor.RegisterSpace = space;
			Descriptor.Flags = flags;
		}
	};

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);

	struct D3D12RootSignatureDesc : public D3D12_ROOT_SIGNATURE_DESC1
	{
		constexpr static D3D12_ROOT_SIGNATURE_FLAGS DefaultFlags{
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
		};

		constexpr explicit D3D12RootSignatureDesc(const D3D12RootParameter* parameters, uint32 parameterCount,
			D3D12_ROOT_SIGNATURE_FLAGS flags = DefaultFlags,
			const D3D12_STATIC_SAMPLER_DESC* staticSamplers = nullptr, uint32 samplerCount = 0)
			: D3D12_ROOT_SIGNATURE_DESC1{parameterCount, parameters, samplerCount, staticSamplers, flags}
		{}

		ID3D12RootSignature* Create() const
		{
			return CreateRootSignature(*this);
		}
	};

#pragma warning(push)
#pragma warning(disable : 4324) // パディングの警告を無効化
	template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
	class alignas(void*) D3D12PipelineStateSubobject
	{
	public:
		D3D12PipelineStateSubobject() = default;

		constexpr explicit D3D12PipelineStateSubobject(T subobject) : _Type{ type }, _Subobject{ subobject }
		{}

		D3D12PipelineStateSubobject& operator=(const T& subobject)
		{
			_Subobject = subobject;
			return *this;
		}

	private:
		const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _Type{ type };
		T _Subobject{};
	};
#pragma warning(pop)

#define PSS(name, ...) using D3D12PipelineStateSubobject##name = D3D12PipelineStateSubobject<__VA_ARGS__>;

	PSS(RootSignature, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*);
	PSS(VS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE);
	PSS(PS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE);
	PSS(DS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS, D3D12_SHADER_BYTECODE);
	PSS(HS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS, D3D12_SHADER_BYTECODE);
	PSS(GS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS, D3D12_SHADER_BYTECODE);
	PSS(CS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, D3D12_SHADER_BYTECODE);
	PSS(StreamOutput, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT, D3D12_STREAM_OUTPUT_DESC);
	PSS(Blend, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, D3D12_BLEND_DESC);
	PSS(SampleMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, uint32);
	PSS(Rasterizer, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, D3D12_RASTERIZER_DESC);
	PSS(DepthStencil, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL, D3D12_DEPTH_STENCIL_DESC);
	PSS(InputLayout, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT, D3D12_INPUT_LAYOUT_DESC);
	PSS(IbStripCutValue, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE, D3D12_INDEX_BUFFER_STRIP_CUT_VALUE);
	PSS(PrimitiveTopology, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE);
	PSS(RenderTargetFormats, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY);
	PSS(DepthStencilFormat, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT);
	PSS(SampleDesc, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC);
	PSS(NodeMask, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, uint32);
	PSS(CachedPso, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO, D3D12_CACHED_PIPELINE_STATE);
	PSS(Flags, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS);
	PSS(DepthStencil1, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, D3D12_DEPTH_STENCIL_DESC1);
	PSS(ViewInstancing, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING, D3D12_VIEW_INSTANCING_DESC);
	PSS(As, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE);
	PSS(Ms, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE);

#undef PSS

	struct D3D12PipelineStateSubobjectStream {
		D3D12PipelineStateSubobjectRootSignature rootSignature{ nullptr };
		D3D12PipelineStateSubobjectVS VS{};
		D3D12PipelineStateSubobjectPS PS{};
		D3D12PipelineStateSubobjectDS DS{};
		D3D12PipelineStateSubobjectHS HS{};
		D3D12PipelineStateSubobjectGS GS{};
		D3D12PipelineStateSubobjectCS CS{};
		D3D12PipelineStateSubobjectStreamOutput StreamOutput{};
		D3D12PipelineStateSubobjectBlend Blend{ BlendState.Disabled };
		D3D12PipelineStateSubobjectSampleMask SampleMask{ UINT_MAX };
		D3D12PipelineStateSubobjectRasterizer Rasterizer{ RasterizerState.NoCull };
		D3D12PipelineStateSubobjectInputLayout InputLayout{};
		D3D12PipelineStateSubobjectIbStripCutValue IbStripCutValue{};
		D3D12PipelineStateSubobjectPrimitiveTopology PrimitiveTopology{};
		D3D12PipelineStateSubobjectRenderTargetFormats RenderTargetFormats{};
		D3D12PipelineStateSubobjectDepthStencilFormat DepthStencilFormat{};
		D3D12PipelineStateSubobjectSampleDesc SampleDesc{ {1, 0} };
		D3D12PipelineStateSubobjectNodeMask NodeMask{};
		D3D12PipelineStateSubobjectCachedPso CachedPso{};
		D3D12PipelineStateSubobjectFlags Flags{};
		D3D12PipelineStateSubobjectDepthStencil1 DepthStencil1{ DepthState.Disabled };
		D3D12PipelineStateSubobjectViewInstancing ViewInstancing{};
		D3D12PipelineStateSubobjectAs AS{};
		D3D12PipelineStateSubobjectMs MS{};
	};

	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc);
	ID3D12PipelineState* CreatePipelineState(void* stream, uint64 stereamSize);

	ID3D12Resource* CreateBuffer(const void* data, uint32 bufferSize, bool isCPU_Accessible = false,
								  D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
								  D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
								  ID3D12Heap* heap = nullptr, uint64 heapOffset = 0);
}