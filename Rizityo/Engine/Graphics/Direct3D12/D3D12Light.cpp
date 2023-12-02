#include "D3D12Light.h"
#include "D3D12Core.h"
#include "Shaders/SharedTypes.h"
#include "EngineAPI/GameEntity.h"

namespace Rizityo::Graphics::D3D12::Light
{
	namespace
	{

		struct LightOwner
		{
			GameEntity::EntityID EntityID{ ID::INVALID_ID };
			uint32 Index;
			Graphics::Light::Type Type;
			bool IsEnabled;
		};

#if USE_STL_VECTOR
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

		class LightSet
		{
		public:

			constexpr Graphics::Light Add(const LightInitInfo& info)
			{
				if (info.Type == Graphics::Light::Directional)
				{
					uint32 index = UINT32_INVALID_NUM;

					// 利用可能なインデックスを見つける
					for (uint32 i = 0; i < _NonCullableOwners.size(); i++)
					{
						if (!ID::IsValid(_NonCullableOwners[i]))
						{
							index = i;
							break;
						}
					}

					// 利用可能なインデックスが無ければ追加
					if (index == UINT32_INVALID_NUM)
					{
						index = (uint32)_NonCullableOwners.size();
						_NonCullableOwners.emplace_back();
						_NonCullableLights.emplace_back();
					}

					HLSL::DirectionalLightParameters& params{ _NonCullableLights[index] };
					params.Color = info.Color;
					params.Intensity = info.Intensity;

					LightOwner owner{ GameEntity::EntityID{info.EntityID}, index, info.Type, info.IsEnabled };
					const LightID id{ _Owners.Add(owner) };
					_NonCullableOwners[index] = id;

					return Graphics::Light{ id, info.LightSetKey };
				}
				else
				{
					return {};
				}
			}

			constexpr void Remove(LightID id)
			{
				SetEnable(id, false);

				const LightOwner& owner{ _Owners[id] };

				if (owner.Type == Graphics::Light::Directional)
				{
					_NonCullableOwners[owner.Index] = LightID{ ID::INVALID_ID };
				}
				else
				{

				}

				_Owners.Remove(id);
			}

			void UpdateTransforms()
			{
				// Directionを更新
				for (const auto& id : _NonCullableOwners)
				{
					if (!ID::IsValid(id))
						continue;

					const LightOwner& owner{ _Owners[id] };
					if (owner.IsEnabled)
					{
						const GameEntity::Entity entity{ GameEntity::EntityID{owner.EntityID} };
						HLSL::DirectionalLightParameters& params{ _NonCullableLights[owner.Index] };
						params.Direction = entity.GetOrientation();
					}
				}

			}

			constexpr void SetEnable(LightID id, bool isEnabled)
			{
				_Owners[id].IsEnabled = isEnabled;

				if (_Owners[id].Type == Graphics::Light::Directional)
				{
					return;
				}


			}

			constexpr void SetIntensity(LightID id, float32 intensity)
			{
				if (intensity < 0.f) intensity = 0.f;

				const LightOwner& owner{ _Owners[id] };
				const uint32 index{ owner.Index };

				if (owner.Type == Graphics::Light::Directional)
				{
					assert(index < _NonCullableLights.size());
					_NonCullableLights[index].Intensity = intensity;
				}
				else
				{

				}
			}

			constexpr void SetColor(LightID id, Math::DX_Vector3 color)
			{
				assert(color.x <= 1.f && color.y <= 1.f && color.z <= 1.f);
				assert(color.x >= 0.f && color.y >= 0.f && color.z >= 0.f);

				const LightOwner& owner{ _Owners[id] };
				const uint32 index{ owner.Index };

				if (owner.Type == Graphics::Light::Directional)
				{
					assert(index < _NonCullableLights.size());
					_NonCullableLights[index].Color = color;
				}
				else
				{

				}
			}

			constexpr bool IsEnabled(LightID id) const
			{
				return _Owners[id].IsEnabled;
			}

			constexpr float32 GetIntensity(LightID id) const
			{
				const LightOwner& owner{ _Owners[id] };
				const uint32 index{ owner.Index };
				if (owner.Type == Graphics::Light::Directional)
				{
					assert(index < _NonCullableLights.size());
					return _NonCullableLights[index].Intensity;
				}


				return 0.f;
			}

			constexpr Math::DX_Vector3 GetColor(LightID id) const
			{
				const LightOwner& owner{ _Owners[id] };
				const uint32 index{ owner.Index };
				if (owner.Type == Graphics::Light::Directional)
				{
					assert(index < _NonCullableLights.size());
					return _NonCullableLights[index].Color;
				}

				return {};
			}

			constexpr Graphics::Light::Type GetType(LightID id) const
			{
				return _Owners[id].Type;
			}

			constexpr ID::IDType GetEntityID(LightID id) const
			{
				return _Owners[id].EntityID;
			}

			// 有効なDirectional Lightの数
			CONSTEXPR uint32 GetNonCullableLightCount() const
			{
				uint32 count = 0;
				for (const auto& id : _NonCullableOwners)
				{
					if (ID::IsValid(id) && _Owners[id].IsEnabled)
						count++;
				}

				return count;
			}

			CONSTEXPR void GetNonCullableLights(OUT HLSL::DirectionalLightParameters* const lights, [[maybe_unused]] uint32 bufferSize)
			{
				assert(bufferSize == Math::AlignSizeUp<D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(GetNonCullableLightCount() * sizeof(HLSL::DirectionalLightParameters)));
				const uint32 count = (uint32)_NonCullableOwners.size();
				uint32 index = 0;
				for (uint32 i = 0; i < count; i++)
				{
					if (!ID::IsValid(_NonCullableOwners[i]))
						continue;

					const LightOwner& owner{ _Owners[_NonCullableOwners[i]] };
					if (owner.IsEnabled)
					{
						assert(_Owners[_NonCullableOwners[i]].Index == i);
						lights[index] = _NonCullableLights[i];
						index++;
					}
				}
			}

			constexpr bool HasLights() const
			{
				return _Owners.Size() > 0;
			}

		private:

			// タイトにパックする必要はない
			Utility::FreeList<LightOwner> _Owners;
			Utility::Vector<HLSL::DirectionalLightParameters> _NonCullableLights;
			Utility::Vector<LightID> _NonCullableOwners;

		};

		class D3D12LightBuffer
		{
		public:

			D3D12LightBuffer() = default;

			CONSTEXPR void UpdateLightBuffers(LightSet& set, uint64 lightSetKey, uint32 frameIndex)
			{
				uint32 sizes[LightBuffer::Count]{};
				sizes[LightBuffer::NonCullableLight] = set.GetNonCullableLightCount() * sizeof(HLSL::DirectionalLightParameters);

				uint32 currentSizes[LightBuffer::Count]{};
				currentSizes[LightBuffer::NonCullableLight] = _Buffers[LightBuffer::NonCullableLight].Buffer.Size();

				// 必要なバッファが今よりも大きいならリサイズ
				if (currentSizes[LightBuffer::NonCullableLight] < sizes[LightBuffer::NonCullableLight])
				{
					ResizeBuffer(LightBuffer::NonCullableLight, sizes[LightBuffer::NonCullableLight], frameIndex);
				}

				set.GetNonCullableLights((HLSL::DirectionalLightParameters* const)_Buffers[LightBuffer::NonCullableLight].CPU_Address,
					_Buffers[LightBuffer::NonCullableLight].Buffer.Size());

			}

			constexpr void Release()
			{
				for (uint32 i = 0; i < LightBuffer::Count; i++)
				{
					_Buffers[i].Buffer.Release();
					_Buffers[i].CPU_Address = nullptr;
				}
			}

			[[nodiscard]] constexpr D3D12_GPU_VIRTUAL_ADDRESS GetNonCullableLights() const
			{
				return _Buffers[LightBuffer::NonCullableLight].Buffer.GPU_Address();
			}

		private:
			struct LightBuffer
			{
				enum Type : uint32 {
					NonCullableLight,
					CullableLight,
					CullingInfo,
					Count
				};

				D3D12Buffer Buffer{};
				uint8* CPU_Address = nullptr;
			};

			void ResizeBuffer(LightBuffer::Type type, uint32 size, [[maybe_unused]] uint32 frameIndex)
			{
				assert(type < LightBuffer::Count);
				if (!size)
					return;

				_Buffers[type].Buffer.Release();
				_Buffers[type].Buffer = D3D12Buffer{ ConstantBuffer::GetDefaultInitInfo(size), true };
				SET_NAME_D3D12_OBJECT_INDEXED(_Buffers[type].Buffer.Buffer(), frameIndex,
					type == LightBuffer::NonCullableLight ? L"Non-cullable Light Buffer" :
					type == LightBuffer::CullableLight ? L"Cullable Light Buffer" : L"Light Culling Info Buffer");

				D3D12_RANGE range{};
				DXCall(_Buffers[type].Buffer.Buffer()->Map(0, &range, (void**)(&_Buffers[type].CPU_Address)));
				assert(_Buffers[type].CPU_Address);
			}

			LightBuffer _Buffers[LightBuffer::Count];
			uint64 _CurrentLightSetKey = 0;
		};

#undef CONSTEXPR

		std::unordered_map<uint64, LightSet> LightSetsMap;
		D3D12LightBuffer LightBuffers[FrameBufferCount];

	} // 変数

	namespace
	{

		constexpr void SetEnable(LightSet& set, LightID id, const void* const data, [[maybe_unused]] uint32 size)
		{
			bool is_enabled{ *(bool*)data };
			assert(sizeof(is_enabled) == size);
			set.SetEnable(id, is_enabled);
		}

		constexpr void SetIntensity(LightSet& set, LightID id, const void* const data, [[maybe_unused]] uint32 size)
		{
			float32 intensity{ *(float32*)data };
			assert(sizeof(intensity) == size);
			set.SetIntensity(id, intensity);
		}

		constexpr void SetColor(LightSet& set, LightID id, const void* const data, [[maybe_unused]] uint32 size)
		{
			Math::DX_Vector3 color{ *(Math::DX_Vector3*)data };
			assert(sizeof(color) == size);
			set.SetColor(id, color);
		}

		constexpr void GetIsEnabled(const LightSet& set, LightID id, OUT void* const data, [[maybe_unused]] uint32 size)
		{
			bool* const is_enabled{ (bool* const)data };
			assert(sizeof(bool) == size);
			*is_enabled = set.IsEnabled(id);
		}

		constexpr void GetIntensity(const LightSet& set, LightID id, OUT void* const data, [[maybe_unused]] uint32 size)
		{
			float32* const intensity{ (float32* const)data };
			assert(sizeof(float32) == size);
			*intensity = set.GetIntensity(id);
		}

		constexpr void GetColor(const LightSet& set, LightID id, OUT void* const data, [[maybe_unused]] uint32 size)
		{
			Math::DX_Vector3* const color{ (Math::DX_Vector3* const)data };
			assert(sizeof(Math::DX_Vector3) == size);
			*color = set.GetColor(id);
		}

		constexpr void GetType(const LightSet& set, LightID id, OUT void* const data, [[maybe_unused]] uint32 size)
		{
			Graphics::Light::Type* const type{ (Graphics::Light::Type* const)data };
			assert(sizeof(Graphics::Light::Type) == size);
			*type = set.GetType(id);
		}

		constexpr void GetEntityID(const LightSet& set, LightID id, OUT void* const data, [[maybe_unused]] uint32 size)
		{
			ID::IDType* const entity_id{ (ID::IDType* const)data };
			assert(sizeof(ID::IDType) == size);
			*entity_id = set.GetEntityID(id);
		}

		constexpr void SetDummy(LightSet&, LightID, const void* const, uint32)
		{}

		using SetFunc = void(*)(LightSet&, LightID, const void* const, uint32);
		using GetFunc = void(*)(const LightSet&, LightID, OUT void* const, uint32);

		constexpr SetFunc SetFunctions[]
		{
			SetEnable,
			SetIntensity,
			SetColor,
			SetDummy,
			SetDummy,
		};

		static_assert(_countof(SetFunctions) == LightParameter::Count);

		constexpr GetFunc GetFunctions[]
		{
			GetIsEnabled,
			GetIntensity,
			GetColor,
			GetType,
			GetEntityID,
		};

		static_assert(_countof(GetFunctions) == LightParameter::Count);

	} // 関数

	bool Initialize()
	{
		return true;
	}

	void Shutdown()
	{
		assert([] {
			bool hasLights = false;
			for (const auto& it : LightSetsMap)
			{
				hasLights |= it.second.HasLights();
			}
			return !hasLights;
			}());

		for (uint32 i = 0; i < FrameBufferCount; i++)
		{
			LightBuffers[i].Release();
		}
	}

	Graphics::Light Create(LightInitInfo info)
	{
		assert(ID::IsValid(info.EntityID));
		return LightSetsMap[info.LightSetKey].Add(info);
	}

	void Remove(LightID id, uint64 lightSetKey)
	{
		assert(LightSetsMap.count(lightSetKey));
		LightSetsMap[lightSetKey].Remove(id);
	}

	void SetParameter(LightID id, uint64 lightSetKey, LightParameter::Parameter parameter, const void* const data, uint32 dataSize)
	{
		assert(data && dataSize);
		assert(LightSetsMap.count(lightSetKey));
		assert(parameter < LightParameter::Count && SetFunctions[parameter] != SetDummy);
		if (parameter < LightParameter::Count && SetFunctions[parameter] != SetDummy)
		{
			SetFunctions[parameter](LightSetsMap[lightSetKey], id, data, dataSize);
		}
	}

	void GetParameter(LightID id, uint64 lightSetKey, LightParameter::Parameter parameter, OUT void* const data, uint32 dataSize)
	{
		assert(data && dataSize);
		assert(LightSetsMap.count(lightSetKey));
		assert(parameter < LightParameter::Count);
		if (parameter < LightParameter::Count)
		{
			GetFunctions[parameter](LightSetsMap[lightSetKey], id, data, dataSize);
		}

	}

	void UpdateLightBuffers(const D3D12FrameInfo& d3d12Info)
	{
		const uint64 lightSetKey = d3d12Info.FrameInfo->LightSetKey;
		assert(LightSetsMap.count(lightSetKey));
		LightSet& set{ LightSetsMap[lightSetKey] };
		if (!set.HasLights())
			return;

		set.UpdateTransforms();
		const uint32 frameIndex = d3d12Info.FrameIndex;
		D3D12LightBuffer& lightBuffer{ LightBuffers[frameIndex] };
		lightBuffer.UpdateLightBuffers(set, lightSetKey, frameIndex);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetNonCullableLightBuffer(uint32 frameIndex)
	{
		const D3D12LightBuffer& lightBuffer{ LightBuffers[frameIndex] };
		return lightBuffer.GetNonCullableLights();
	}

	uint32 GetNonCullableLightCount(uint64 lightSetKey)
	{
		assert(LightSetsMap.count(lightSetKey));
		return LightSetsMap[lightSetKey].GetNonCullableLightCount();
	}

}