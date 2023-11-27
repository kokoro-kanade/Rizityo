#include "D3D12Shader.h"
#include "Content/ContentLoader.h"
#include "Content/ContentToEngine.h"

namespace Rizityo::Graphics::D3D12::Shader
{
	namespace
	{
		// 各要素はShadersBlob内の位置を表す
		Content::CompiledShaderPtr EngineShaders[EngineShader::Count]{};

		// すべてのコンパイルされたエンジンシェーダーのメモリ
		// 一つ一つのシェーダーはバイトコードのサイズとハッシュ列の長さとそれに続くバイトコードからなる
		std::unique_ptr<uint8[]> EngineShadersBlob{};

		bool LoadEngineShaders()
		{
			assert(!EngineShadersBlob);

			uint64 size = 0;
			bool result = Content::LoadEngineShaders(EngineShadersBlob, size);
			assert(EngineShadersBlob && size);

			uint64 offset = 0;
			uint32 index = 0;
			while (offset < size && result)
			{
				assert(index < EngineShader::Count);

				Content::CompiledShaderPtr& shader{ EngineShaders[index] };
				assert(!shader);

				result &= index < EngineShader::Count && !shader;
				if (!result)
					break;

				shader = reinterpret_cast<const Content::CompiledShaderPtr>(&EngineShadersBlob[offset]);
				offset += shader->BufferSize();
				index++;
			}
			assert(offset == size && index == EngineShader::Count);

			return result;
		}

	} // 無名空間

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
		EngineShadersBlob.reset();
	}

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id)
	{
		assert(id < EngineShader::Count);
		const Content::CompiledShaderPtr& shader = EngineShaders[id];
		assert(shader && shader->ByteCodeSize());
		return { shader->ByteCode(), shader->ByteCodeSize()};
	}
}