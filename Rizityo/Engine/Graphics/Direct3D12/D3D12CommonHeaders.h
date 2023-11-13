#pragma once
#include "CommonHeaders.h"
#include "Graphics/Renderer.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")


namespace Rizityo::Graphics::D3D12
{
	constexpr uint32 FrameBufferCount = 3;
}

// Rename: DXCall
#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)								\
if (FAILED(x))									\
{												\
	char lineNum[32];							\
	sprintf_s(lineNum, "%u", __LINE__);			\
	OutputDebugStringA("�G���[�t�@�C��: ");		\
	OutputDebugStringA(__FILE__);				\
	OutputDebugStringA("\n�s: ");			    \
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
OutputDebugString(L"::D3D12�I�u�W�F�N�g���쐬����܂���: ");	\
OutputDebugString(name);										\
OutputDebugString(L"\n");
#define SET_NAME_D3D12_OBJECT_INDEXED(obj, index, name)					 \
{																		 \
	wchar_t fullname[128];												 \
	if(swprintf_s(fullname, L"%s[%u]", name, index) > 0)				 \
	{																	 \
		obj->SetName(fullname);											 \
		OutputDebugString(L"::D3D12�I�u�W�F�N�g���쐬����܂���: ");	 \
		OutputDebugString(fullname);									 \
		OutputDebugString(L"\n");										 \
	}																	 \
}
#else
#define SET_NAME_D3D12_OBJECT(obj, name)
#define SET_NAME_D3D12_OBJECT_INDEXED(obj, index, name)
#endif // _DEBUG
