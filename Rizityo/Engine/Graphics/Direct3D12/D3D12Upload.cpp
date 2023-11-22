#include "D3D12Upload.h"
#include "D3D12Core.h"

namespace Rizityo::Graphics::D3D12::Upload
{
    namespace
    {
        struct UploadFrame
        {
            ID3D12CommandAllocator* CmdAllocator = nullptr;
            ID3D12GraphicsCommandList* CmdList = nullptr;
            ID3D12Resource* UploadBuffer = nullptr;
            void* CPU_Address = nullptr;
            uint64 FenceValue = 0;

            void WaitAndReset();

            void Release()
            {
                WaitAndReset();
                Core::Release(CmdAllocator);
                Core::Release(CmdList);
            }

            constexpr bool IsReady() const { return UploadBuffer == nullptr; }
        };

        constexpr uint32 UploadFrameCount = 4;
        UploadFrame UploadFrames[UploadFrameCount]{};
        ID3D12CommandQueue* UploadCmdQueue = nullptr;
        ID3D12Fence1* UploadFence = nullptr;
        uint64 UploadFenceValue = 0;
        HANDLE FenceEvent{};
        std::mutex FrameMutex{};
        std::mutex QueueMutex{};

    } // 変数

    namespace
    {
        void UploadFrame::WaitAndReset()
        {
            assert(UploadFence && FenceEvent);
            if (UploadFence->GetCompletedValue() < FenceValue)
            {
                DXCall(UploadFence->SetEventOnCompletion(FenceValue, FenceEvent));
                WaitForSingleObject(FenceEvent, INFINITE);
            }

            Core::Release(UploadBuffer);
            CPU_Address = nullptr;
        }

        // この関数が呼ばれる前にフレームがロックされているべき
        uint32 GetAvailableUploadFrame()
        {
            uint32 index = UINT32_INVALID_NUM;
            const uint32 count = UploadFrameCount;
            UploadFrame* const frames = &UploadFrames[0];

            for (uint32 i = 0; i < count; i++)
            {
                if (frames[i].IsReady())
                {
                    index = i;
                    break;
                }
            }

            // フレームが準備できるまで待機
            if (index == UINT32_INVALID_NUM)
            {
                index = 0;
                while (!frames[index].IsReady())
                {
                    index = (index + 1) % count;
                    std::this_thread::yield();
                }
            }

            return index;
        }

        bool FailedInit()
        {
            Shutdown();
            return false;
        }

    } // 関数

    D3D12UploadContext::D3D12UploadContext(uint32 alignedSize)
    {
        assert(UploadCmdQueue);
        {
            std::lock_guard lock{ FrameMutex };
            _FrameIndex = GetAvailableUploadFrame();
            assert(_FrameIndex != UINT32_INVALID_NUM);
            // lockを開放する前にほかのスレッドが参照しないようにIsReadyがfalseになるようにする
            UploadFrames[_FrameIndex].UploadBuffer = (ID3D12Resource*)1;
        }

        UploadFrame& frame{ UploadFrames[_FrameIndex] };
        frame.UploadBuffer = Helper::CreateBuffer(nullptr, alignedSize, true);
        SET_NAME_D3D12_OBJECT_INDEXED(frame.UploadBuffer, alignedSize, L"Upload Buffer - size");

        const D3D12_RANGE range{};
        DXCall(frame.UploadBuffer->Map(0, &range, reinterpret_cast<void**>(&frame.CPU_Address)));
        assert(frame.CPU_Address);

        _CmdList = frame.CmdList;
        _UploadBuffer = frame.UploadBuffer;
        _CPU_Address = frame.CPU_Address;
        assert(_CmdList && _UploadBuffer && _CPU_Address);

        DXCall(frame.CmdAllocator->Reset());
        DXCall(frame.CmdList->Reset(frame.CmdAllocator, nullptr));
    }

    void D3D12UploadContext::EndUpload()
    {
        assert(_FrameIndex != UINT32_INVALID_NUM);
        UploadFrame& frame{ UploadFrames[_FrameIndex] };
        ID3D12GraphicsCommandList* const cmdList = frame.CmdList;
        DXCall(cmdList->Close());

        std::lock_guard lock{ QueueMutex };

        ID3D12CommandList* const cmdLists[]{ cmdList };
        ID3D12CommandQueue* const cmdQueue = UploadCmdQueue;
        cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

        UploadFenceValue++;
        frame.FenceValue = UploadFenceValue;
        DXCall(cmdQueue->Signal(UploadFence, frame.FenceValue));

        // キューのコピーが終わるのを待った後、UploadBufferを解放
        frame.WaitAndReset();
        // このインスタンスを使えないように
        DEBUG_ONLY(new (this) D3D12UploadContext{});
    }

    bool Initialize()
    {
        ID3D12Device* const device = Core::GetMainDevice();
        assert(device && !UploadCmdQueue);

        HRESULT hr{ S_OK };

        for (uint32 i = 0; i < UploadFrameCount; i++)
        {
            UploadFrame& frame{ UploadFrames[i] };
            DXCall(hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&frame.CmdAllocator)));
            if (FAILED(hr))
                return FailedInit();

            DXCall(hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, frame.CmdAllocator, nullptr, IID_PPV_ARGS(&frame.CmdList)));
            if (FAILED(hr))
                return FailedInit();

            DXCall(frame.CmdList->Close());

            SET_NAME_D3D12_OBJECT_INDEXED(frame.CmdAllocator, i, L"Upload Command Allocator");
            SET_NAME_D3D12_OBJECT_INDEXED(frame.CmdList, i, L"Upload Command List");
        }

        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

        DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&UploadCmdQueue)));
        if (FAILED(hr))
            return FailedInit();

        SET_NAME_D3D12_OBJECT(UploadCmdQueue, L"Upload Copy Queue");

        DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&UploadFence)));
        if (FAILED(hr))
            return FailedInit();

        SET_NAME_D3D12_OBJECT(UploadFence, L"Upload Fence");

        FenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(FenceEvent);
        if (!FenceEvent)
            return FailedInit();

        return true;
    }

    void Shutdown()
    {
        for (uint32 i = 0; i < UploadFrameCount; i++)
        {
            UploadFrames[i].Release();
        }

        if (FenceEvent)
        {
            CloseHandle(FenceEvent);
            FenceEvent = nullptr;
        }

        Core::Release(UploadCmdQueue);
        Core::Release(UploadFence);
        UploadFenceValue = 0;
    }

}