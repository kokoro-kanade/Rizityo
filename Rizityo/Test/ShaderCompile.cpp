#include <d3d12shader.h>
#include <dxcapi.h>
#include "ShaderCompile.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Graphics/Direct3D12/D3D12Shader.h"
#include <fstream>
#include <filesystem>
#include "Content/ContentToEngine.h"
#include "Utility/IOStream.h"

#pragma comment(lib, "dxcompiler")

using namespace Rizityo;
using namespace Rizityo::Graphics::D3D12::Shader;
using namespace Microsoft::WRL;

namespace
{
	struct EngineShaderInfo
	{
		EngineShader::ID ID;
		ShaderFileInfo Info;
	};

	constexpr EngineShaderInfo EngineShaderFiles[]
	{
		EngineShader::FullScreenTriangleVS, {"FullScreenTriangle.hlsl", "FullScreenTriangleVS", ShaderType::Vertex},
		EngineShader::FillColorPS, {"FillColor.hlsl", "FillColorPS", ShaderType::Pixel},
		EngineShader::PostProcessPS, {"PostProcess.hlsl", "PostProcessPS", ShaderType::Pixel},
	};

	static_assert(_countof(EngineShaderFiles) == EngineShader::Count);

	constexpr const char* ShadersSourcePath = "../../Engine/Graphics/Direct3D12/Shaders/";

	struct DxcCompiledShader
	{
		ComPtr<IDxcBlob>        ByteCode;
		ComPtr<IDxcBlobUtf8>    Disassembly;
		DxcShaderHash           Hash;
	};

	std::wstring ToWstring(const char* c)
	{
		std::string s{ c };
		return { s.begin(), s.end() };
	}

	class ShaderCompiler
	{
	public:
		ShaderCompiler()
		{
			HRESULT hr{ S_OK };
			DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_Compiler)));
			if (FAILED(hr))
				return;
			DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_Utils)));
			if (FAILED(hr))
				return;
			DXCall(hr = _Utils->CreateDefaultIncludeHandler(&_IncludeHandler));
			if (FAILED(hr))
				return;
		}

		DISABLE_COPY_AND_MOVE(ShaderCompiler);

		DxcCompiledShader Compile(ShaderFileInfo info, std::filesystem::path fullPath)
		{
			assert(_Compiler && _Utils && _IncludeHandler);
			HRESULT hr{ S_OK };

			ComPtr<IDxcBlobEncoding> sourceBlob{ nullptr };
			DXCall(hr = _Utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
			if (FAILED(hr))
				return {};
			assert(sourceBlob && sourceBlob->GetBufferSize());

			std::wstring file{ ToWstring(info.FileName) };
			std::wstring func{ ToWstring(info.FunctionName) };
			std::wstring prof{ ToWstring(_profileStrings[(uint32)info.Type]) };
			std::wstring inc{ ToWstring(ShadersSourcePath) };

			LPCWSTR args[]
			{
				file.c_str(),						 // エラーレポート用のファイル名
				L"-E", func.c_str(),				 // エントリーファンクション
				L"-T", prof.c_str(),				 // ターゲットプロファイル
				L"-I", inc.c_str(),					 // インクルードパス
				L"-enable-16bit-types",
				DXC_ARG_ALL_RESOURCES_BOUND,
#if _DEBUG
				DXC_ARG_DEBUG,
				DXC_ARG_SKIP_OPTIMIZATIONS,
#else
				DXC_ARG_OPTIMIZATION_LEVEL3,
#endif // _DEBUG
				DXC_ARG_WARNINGS_ARE_ERRORS,
				L"-Qstrip_reflect",
				L"-Qstrip_debug",
			};

			OutputDebugStringA("コンパイル ");
			OutputDebugStringA(info.FileName);
			OutputDebugStringA(" : ");
			OutputDebugStringA(info.FunctionName);
			OutputDebugStringA("\n");

			return Compile(sourceBlob.Get(), args, _countof(args));
		}

		DxcCompiledShader Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs)
		{
			DxcBuffer buffer{};
			buffer.Encoding = DXC_CP_ACP;
			buffer.Ptr = sourceBlob->GetBufferPointer();
			buffer.Size = sourceBlob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(hr = _Compiler->Compile(&buffer, args, numArgs, _IncludeHandler.Get(), IID_PPV_ARGS(&results)));
			if (FAILED(hr))
				return {};

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
			if (FAILED(hr))
				return {};

			if (errors && errors->GetStringLength())
			{
				OutputDebugStringA("\nシェーダーコンパイルエラー: \n");
				OutputDebugStringA(errors->GetStringPointer());
			}
			else
			{
				OutputDebugStringA(" [ 成功 ]");
			}
			OutputDebugStringA("\n");

			HRESULT status{ S_OK };
			DXCall(hr = results->GetStatus(&status));
			if (FAILED(hr))
				return {};

			ComPtr<IDxcBlob> hash{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&hash), nullptr));
			if (FAILED(hr))
				return {};

			DxcShaderHash* const hashBuffer{ (DxcShaderHash* const)hash->GetBufferPointer() };
			assert(!(hashBuffer->Flags & DXC_HASHFLAG_INCLUDES_SOURCE));

			OutputDebugStringA("Shader hash: ");
			for (uint32 i{ 0 }; i < _countof(hashBuffer->HashDigest); ++i)
			{
				char hash_bytes[3]{}; // 2 chars for hex value plus termination 0.
				sprintf_s(hash_bytes, "%02x", (uint32)hashBuffer->HashDigest[i]);
				OutputDebugStringA(hash_bytes);
				OutputDebugStringA(" ");
			}
			OutputDebugStringA("\n");

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr))
				return {};

			buffer.Ptr = shader->GetBufferPointer();
			buffer.Size = shader->GetBufferSize();

			ComPtr<IDxcResult> disasmResults{ nullptr };
			DXCall(hr = _Compiler->Disassemble(&buffer, IID_PPV_ARGS(&disasmResults)));

			ComPtr<IDxcBlobUtf8> disassembly{ nullptr };
			DXCall(hr = disasmResults->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr));

			DxcCompiledShader result{ shader.Detach(), disassembly.Detach() };
			memcpy(&result.Hash.HashDigest[0], &hashBuffer->HashDigest[0], _countof(hashBuffer->HashDigest));

			return result;
		}

	private:
		constexpr static const char* _profileStrings[ShaderType::Count]{ "vs_6_6", "hs_6_6" , "ds_6_6" , "gs_6_6" , "ps_6_6" , "cs_6_6", "as_6_6", "ms_6_6" };
		static_assert(_countof(_profileStrings) == ShaderType::Count);

		ComPtr<IDxcCompiler3> _Compiler{ nullptr };
		ComPtr<IDxcUtils> _Utils{ nullptr };
		ComPtr<IDxcIncludeHandler> _IncludeHandler{ nullptr };
	};

	decltype(auto) GetEngineShadersPath()
	{
		return std::filesystem::path{ Graphics::GetEngineShadersPath(Graphics::GraphicsPlatform::Direct3D12) };
	}

	bool CompiledShadersAreUpToDate()
	{
		auto engineShadersPath = GetEngineShadersPath();
		if (!std::filesystem::exists(engineShadersPath))
			return false;

		auto shadersCompileTime = std::filesystem::last_write_time(engineShadersPath);

		std::filesystem::path fullPath{};
		for (uint32 i = 0; i < EngineShader::Count; i++)
		{
			auto& file = EngineShaderFiles[i];

			fullPath = ShadersSourcePath;
			fullPath += file.Info.FileName;
			if (!std::filesystem::exists(fullPath))
				return false;

			auto shaderFileTime = std::filesystem::last_write_time(fullPath);
			if (shaderFileTime > shadersCompileTime)
				return false;
		}

		return true;
	}

	bool SaveCompiledShaders(Utility::Vector<DxcCompiledShader>& shaders)
	{
		auto engineShadersPath = GetEngineShadersPath();
		std::filesystem::create_directories(engineShadersPath.parent_path());
		std::ofstream file(engineShadersPath, std::ios::out | std::ios::binary);
		if (!file || !std::filesystem::exists(engineShadersPath))
		{
			file.close();
			return false;
		}

		for (auto& shader : shaders)
		{
			const D3D12_SHADER_BYTECODE byteCode{ shader.ByteCode->GetBufferPointer(), shader.ByteCode->GetBufferSize() };
			file.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
			file.write((char*)&shader.Hash.HashDigest[0], _countof(shader.Hash.HashDigest));
			file.write((char*)byteCode.pShaderBytecode, byteCode.BytecodeLength);
		}

		file.close();
		return true; 
	}
}

std::unique_ptr<uint8[]>
CompileShader(ShaderFileInfo info, const char* filePath)
{
	std::filesystem::path full_path{ filePath };
	full_path += info.FileName;
	if (!std::filesystem::exists(full_path)) return {};

	ShaderCompiler compiler{};
	DxcCompiledShader compiled_shader{ compiler.Compile(info, full_path) };

	if (compiled_shader.ByteCode && compiled_shader.ByteCode->GetBufferPointer() && compiled_shader.ByteCode->GetBufferSize())
	{
		static_assert(Content::CompiledShader::HashLength == _countof(DxcShaderHash::HashDigest));
		const uint64 buffer_size{ sizeof(uint64) + Content::CompiledShader::HashLength + compiled_shader.ByteCode->GetBufferSize() };
		std::unique_ptr<uint8[]> buffer{ std::make_unique<uint8[]>(buffer_size) };
		Utility::BinaryWriter writer{ buffer.get(), buffer_size };
		writer.Write(compiled_shader.ByteCode->GetBufferSize());
		writer.Write(compiled_shader.Hash.HashDigest, Content::CompiledShader::HashLength);
		writer.Write((uint8*)compiled_shader.ByteCode->GetBufferPointer(), compiled_shader.ByteCode->GetBufferSize());;

		assert(writer.offset() == buffer_size);
		return buffer;
	}

	return {};
}

bool CompileShaders()
{
	if (CompiledShadersAreUpToDate())
		return true;

	ShaderCompiler compiler{};
	Utility::Vector<DxcCompiledShader> shaders;
	std::filesystem::path fullPath{};

	for (uint32 i = 0; i < EngineShader::Count; i++)
	{
		auto& file = EngineShaderFiles[i];

		fullPath = ShadersSourcePath;
		fullPath += file.Info.FileName;
		if (!std::filesystem::exists(fullPath)) 
			return false;
		DxcCompiledShader compiledShader{ compiler.Compile(file.Info, fullPath) };
		if (compiledShader.ByteCode && compiledShader.ByteCode->GetBufferPointer() && compiledShader.ByteCode->GetBufferSize())
		{
			shaders.emplace_back(std::move(compiledShader));
		}
		else
		{
			return false;
		}
	}

	return SaveCompiledShaders(shaders);
}