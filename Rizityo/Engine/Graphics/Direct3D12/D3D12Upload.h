#pragma once
#include "D3D12CommonHeaders.h"

namespace Rizityo::Graphics::D3D12::Upload
{

    class D3D12UploadContext
    {
    public:
        D3D12UploadContext(uint32 alignedSize);

        DISABLE_COPY_AND_MOVE(D3D12UploadContext);

        ~D3D12UploadContext() { assert(_FrameIndex == UINT32_INVALID_NUM); }

        void EndUpload();

        [[nodiscard]] constexpr ID3D12GraphicsCommandList* const CommandList() const { return _CmdList; }
        [[nodiscard]] constexpr ID3D12Resource* const UploadBuffer() const { return _UploadBuffer; }
        [[nodiscard]] constexpr void* const CPU_Address() const { return _CPU_Address; }

    private:
        DEBUG_ONLY(D3D12UploadContext() = default);

        ID3D12GraphicsCommandList* _CmdList = nullptr;
        ID3D12Resource* _UploadBuffer = nullptr;
        void* _CPU_Address = nullptr;
        uint32 _FrameIndex = UINT32_INVALID_NUM;
    };

    bool Initialize();
    void Shutdown();

}