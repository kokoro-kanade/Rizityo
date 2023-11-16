#include <d3d12shader.h>
#include <dxcapi.h>
#include "ShaderCompile.h"
#include "Graphics/Direct3D12/D3D12Core.h"
#include "Graphics/Direct3D12/D3D12Shader.h"
#include <fstream>
#include <filesystem>

#pragma comment(lib, "dxcompiler")

using namespace Rizityo;
using namespace Rizityo::Graphics::D3D12::Shader;
using namespace Microsoft::WRL;

namespace
{
	struct ShaderFileInfo
	{
		const char* File;
		const char* Function;
		EngineShader::ID ID;
		ShaderType::Type Type;
	};

	constexpr ShaderFileInfo ShaderFiles[]
	{
		{"FullScreenTriangle.hlsl", "FullScreenTriangleVS", EngineShader::FullScreenTriangleVS, ShaderType::Vertex},
		{"FillColor.hlsl", "FillColorPS", EngineShader::FillColorPS, ShaderType::Pixel},
		{"PostProcess.hlsl", "PostProcessPS", EngineShader::PostProcessPS, ShaderType::Pixel},
	};

	static_assert(_countof(ShaderFiles) == EngineShader::Count);

	constexpr const char* ShadersSourcePath = "../../Engine/Graphics/Direct3D12/Shaders/";

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

		IDxcBlob* Compile(ShaderFileInfo info, std::filesystem::path fullPath)
		{
			assert(_Compiler && _Utils && _IncludeHandler);
			HRESULT hr{ S_OK };

			ComPtr<IDxcBlobEncoding> sourceBlob{ nullptr };
			DXCall(hr = _Utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
			if (FAILED(hr))
				return nullptr;
			assert(sourceBlob && sourceBlob->GetBufferSize());

			std::wstring file{ ToWstring(info.File) };
			std::wstring func{ ToWstring(info.Function) };
			std::wstring prof{ ToWstring(_profileStrings[(uint32)info.Type]) };
			std::wstring inc{ ToWstring(ShadersSourcePath) };

			LPCWSTR args[]
			{
				file.c_str(),						 // エラーレポート用のファイル名
				L"-E", func.c_str(),				 // エントリーファンクション
				L"-T", prof.c_str(),				 // ターゲットプロファイル
				L"-I", inc.c_str(),					 // インクルードパス
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
			OutputDebugStringA(info.File);

			return Compile(sourceBlob.Get(), args, _countof(args));
		}

		IDxcBlob* Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32 numArgs)
		{
			DxcBuffer buffer{};
			buffer.Encoding = DXC_CP_ACP;
			buffer.Ptr = sourceBlob->GetBufferPointer();
			buffer.Size = sourceBlob->GetBufferSize();

			HRESULT hr{ S_OK };
			ComPtr<IDxcResult> results{ nullptr };
			DXCall(hr = _Compiler->Compile(&buffer, args, numArgs, _IncludeHandler.Get(), IID_PPV_ARGS(&results)));
			if (FAILED(hr))
				return nullptr;

			ComPtr<IDxcBlobUtf8> errors{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
			if (FAILED(hr))
				return nullptr;

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
				return nullptr;

			ComPtr<IDxcBlob> shader{ nullptr };
			DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
			if (FAILED(hr))
				return nullptr;

			return shader.Detach();
		}

	private:
		constexpr static const char* _profileStrings[ShaderType::Count]{ "vs_6_5", "hs_6_5" , "ds_6_5" , "gs_6_5" , "ps_6_5" , "cs_6_5", "as_6_5", "ms_6_5" };
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

		std::filesystem::path path{};
		for (uint32 i = 0; i < EngineShader::Count; i++)
		{
			auto& info = ShaderFiles[i];

			path = ShadersSourcePath;
			path += info.File;
			if (!std::filesystem::exists(path))
				return false;

			auto shaderFileTime = std::filesystem::last_write_time(path);
			if (shaderFileTime > shadersCompileTime)
				return false;
		}

		return true;
	}

	bool SaveCompiledShaders(Utility::Vector<ComPtr<IDxcBlob>>& shaders)
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
			const D3D12_SHADER_BYTECODE byteCode{ shader->GetBufferPointer(), shader->GetBufferSize() };
			file.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
			file.write((char*)byteCode.pShaderBytecode, byteCode.BytecodeLength);
		}

		file.close();
		return true; 
	}
}

bool CompileShaders()
{
	if (CompiledShadersAreUpToDate())
		return true;

	Utility::Vector<ComPtr<IDxcBlob>> shaders;
	std::filesystem::path path{};
	std::filesystem::path fullPath{};

	ShaderCompiler compiler{};

	for (uint32 i = 0; i < EngineShader::Count; i++)
	{
		auto& info = ShaderFiles[i];

		path = ShadersSourcePath;
		path += info.File;
		fullPath = std::filesystem::absolute(path);
		if (!std::filesystem::exists(fullPath)) 
			return false;
		ComPtr<IDxcBlob> compiledShader{ compiler.Compile(info, fullPath) };
		if (compiledShader != nullptr && compiledShader->GetBufferPointer() && compiledShader->GetBufferSize())
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