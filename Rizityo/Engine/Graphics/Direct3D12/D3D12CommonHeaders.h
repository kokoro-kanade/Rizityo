#pragma once
#include "CommonHeaders.h"
#include "Graphics/Renderer.h"

// windows.hのmin/maxマクロを無効化
#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX


#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")


namespace Rizityo::Graphics::D3D12
{
	constexpr uint32 FrameBufferCount = 3;
	using ID3D12Device = ID3D12Device9;
	using ID3D12GraphicsCommandList = ID3D12GraphicsCommandList6;
}

// Rename: DXCall
#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)								\
if (FAILED(x))									\
{												\
	char lineNum[32];							\
	sprintf_s(lineNum, "%u", __LINE__);			\
	OutputDebugStringA("エラーファイル: ");		\
	OutputDebugStringA(__FILE__);				\
	OutputDebugStringA("\n行: ");			    \
	OutputDebugStringA(lineNum);				\
	OutputDebugStringA("\n");					\
	OutputDebugStringA(#x);						\
	OutputDebugStringA("\n");					\
	__debugbreak();								\
}
#endif // !DXCall
#else
#ifndef DXCall
#define DXCall(x) x
#endif // !DXCall
#endif // _DEBUG

#ifdef _DEBUG													
#define SET_NAME_D3D12_OBJECT(obj, name) 						\
obj->SetName(name);												\
OutputDebugString(L"::D3D12オブジェクトが作成されました: ");	\
OutputDebugString(name);										\
OutputDebugString(L"\n");
#define SET_NAME_D3D12_OBJECT_INDEXED(obj, index, name)					 \
{																		 \
	wchar_t fullname[128];												 \
	if(swprintf_s(fullname, L"%s[%llu]", name, (uint64)index) > 0)		 \
	{																	 \
		obj->SetName(fullname);											 \
		OutputDebugString(L"::D3D12オブジェクトが作成されました: ");	 \
		OutputDebugString(fullname);									 \
		OutputDebugString(L"\n");										 \
	}																	 \
}
#else
#define SET_NAME_D3D12_OBJECT(obj, name)
#define SET_NAME_D3D12_OBJECT_INDEXED(obj, index, name)
#endif // _DEBUG

#include "D3D12Helper.h"
#include "D3D12Resource.h"
