#include "D3D12Shader.h"
#include "Content/ContentLoader.h"

namespace Rizityo::Graphics::D3D12::Shader
{
	namespace
	{
		typedef struct CompiledShader
		{
			uint64 Size;
			const uint8* ByteCode;
		} const* CompiledShaderPtr;

		// �e�v�f��ShadersBlob���̈ʒu��\��
		CompiledShaderPtr EngineShaders[EngineShader::Count]{};

		// ���ׂẴR���p�C�����ꂽ�G���W���V�F�[�_�[�̃�����
		// ���̃V�F�[�_�[�̓T�C�Y�Ƃ���ɑ����o�C�g�R�[�h����Ȃ�
		std::unique_ptr<uint8[]> ShadersBlob{};

		bool LoadEngineShaders()
		{
			assert(!ShadersBlob);

			uint64 size = 0;
			bool result = Content::LoadEngineShaders(ShadersBlob, size);
			assert(ShadersBlob && size);

			uint64 offset = 0;
			uint32 index = 0;
			while (offset < size && result)
			{
				assert(index < EngineShader::Count);

				CompiledShaderPtr& shader{ EngineShaders[index] };
				assert(!shader);

				result &= index < EngineShader::Count && !shader;
				if (!result)
					break;

				shader = reinterpret_cast<const CompiledShaderPtr>(&ShadersBlob[offset]);
				offset += sizeof(uint64) + shader->Size;
				index++;
			}
			assert(offset == size && index == EngineShader::Count);

			return result;
		}

	} // �������

	bool Initialize()
	{
		return LoadEngineShaders();
	}

	void Shutdown()
	{
		for (uint32 i = 0; i < EngineShader::Count; i++)
		{
			EngineShaders[i] = {};
		}
		ShadersBlob.reset();
	}

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id)
	{
		assert(id < EngineShader::Count);
		const CompiledShaderPtr shader = EngineShaders[id];
		assert(shader && shader->Size);
		return { &shader->ByteCode, shader->Size };
	}
}