#include "D3D12Upload.h"
#include "D3D12Core.h"

//memo:
//CPUメモリからGPUメモリへのデータ転送を行うため処理
//1. CPUメモリにデータを渡す
//2. GPUメモリにデータをコピーする
//3. CPUメモリを解放する
//GPUメモリにデータを渡す理由は、shaderでメモリからデータを読み取る際に、CPUメモリからアクセスするよりも高速にアクセスできるため

//TODO: 各オブジェクトの生成を専用のクラスにラッピングする
namespace RHI::D3D12::UPLOAD {
    namespace {
        struct UploadFrame
        {
            ID3D12CommandAllocator* cmd_allocator{ nullptr };
            ID3D12GraphicsCommandList6* cmd_list{ nullptr };
            ID3D12Resource* upload_buffer{ nullptr };
            void* cpu_address{ nullptr };
            u64 fence_value{ 0 };

            void WaitAndReset();

            void Release()
            {
                WaitAndReset();
                if (cmd_allocator != nullptr) {
                    cmd_allocator->Release();
                    cmd_allocator = nullptr;
                }
                if (cmd_list != nullptr) {
                    cmd_list->Release();
                    cmd_list = nullptr;
                }
            }

            constexpr bool IsReady() const { return upload_buffer == nullptr; }
        };

        UploadFrame         upload_frames[upload_frame_count]{};
        ID3D12CommandQueue* upload_cmd_queue{ nullptr };
        ID3D12Fence1* upload_fence{ nullptr };
        u64                 upload_fence_value{ 0 };
        HANDLE              fence_event{};
        std::mutex          frame_mutex{};
        std::mutex          queue_mutex{};

        void UploadFrame::WaitAndReset()
        {
            assert(upload_fence);
            assert(fence_event);

            if (upload_fence->GetCompletedValue() < fence_value) {
                DXCall(upload_fence->SetEventOnCompletion(fence_value, fence_event));
                WaitForSingleObject(fence_event, INFINITE);
            }

            if (upload_buffer != nullptr) {
                upload_buffer->Release();
                upload_buffer = nullptr;
            }
            cpu_address = nullptr;
        }

        // Note：この関数が呼ばれる前に、フレームはロックされていなければならない。
        // 使用可能なアップロードフレームを取得
        u32 GetAvailableUploadFrame()
        {
            u32 index{ u32_invalid_id };
            const u32 count{ upload_frame_count };
            UploadFrame* const frames{ &upload_frames[0] };

            // アップロードが終わっているフレームを探す
            for (u32 i{ 0 }; i < count; ++i) {
                if (frames[i].IsReady()) {
                    index = i;
                    break;
                }
            }

            // すべてのフレームが使用中の場合、最初に終わるフレームが現れるまでloop
            if (index == u32_invalid_id) {
                index = 0;
                while (!frames[index].IsReady()) {
                    index = (index + 1) % count;
                    std::this_thread::yield();
                }
            }

            return index;
        }

        bool InitFailed()
        {
            Shutdown();
            return false;
        }
    }


    D3D12UploadContext::D3D12UploadContext(u32 aligned_size)
    {
        {
            // この関数を必要以上に長くロックしたくないので、このロックをスコープに収めている
            std::lock_guard lock{ frame_mutex };
            frame_index_ = GetAvailableUploadFrame();
            assert(frame_index_ != u32_invalid_id);
            // ロックを解除する前に、is_readyがfalseを返すようにすることで、
            // 他のスレッドがこのフレームをピックできないようにする。
            upload_frames[frame_index_].upload_buffer = (ID3D12Resource*)1;
        }

        UploadFrame& frame{ upload_frames[frame_index_] };
        frame.upload_buffer = D3DX::CreateBuffer(nullptr, aligned_size, true);
        NAME_D3D12_OBJECT_INDEXED(frame.upload_buffer, aligned_size, L"Upload Buffer - size");

        const D3D12_RANGE range{};
        DXCall(frame.upload_buffer->Map(0, &range, reinterpret_cast<void**>(&frame.cpu_address)));
        assert(frame.cpu_address);

        cmd_list_ = frame.cmd_list;
        upload_buffer_ = frame.upload_buffer;
        cpu_address_ = frame.cpu_address;
        assert(cmd_list_ && upload_buffer_ && cpu_address_);

        DXCall(frame.cmd_allocator->Reset());
        DXCall(frame.cmd_list->Reset(frame.cmd_allocator, nullptr));
    }

    void D3D12UploadContext::EndUpload()
    {
        assert(frame_index_ != u32_invalid_id);
        UploadFrame& frame{ upload_frames[frame_index_] };
        ID3D12GraphicsCommandList6* const cmd_list{ frame.cmd_list };
        DXCall(cmd_list->Close());

        std::lock_guard lock{ queue_mutex };

        ID3D12CommandList* const cmd_lists[]{ cmd_list };
        ID3D12CommandQueue* const cmd_queue{ upload_cmd_queue };
        cmd_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

        ++upload_fence_value;
        frame.fence_value = upload_fence_value;
        DXCall(cmd_queue->Signal(upload_fence, frame.fence_value));

        // コピーキューが終了するまで待つ。その後、アップロードバッファを解放
        frame.WaitAndReset();
        // このアップロードコンテキストのインスタンスは期限切れです。二度と使用しないようにしてください。
        DEBUG_OP(new (this) D3D12UploadContext{});
    }

    bool Initialize()
    {
        auto* const device{ Core::GetDevice() };
        assert(device && !upload_cmd_queue);

        HRESULT hr{ S_OK };

        for (u32 i{ 0 }; i < upload_frame_count; ++i)
        {
            //create cmd allocator
            UploadFrame& frame{ upload_frames[i] };
            DXCall(hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&frame.cmd_allocator)));
            if (FAILED(hr)) return InitFailed();

            //create cmd list
            DXCall(hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, frame.cmd_allocator, nullptr, IID_PPV_ARGS(&frame.cmd_list)));
            if (FAILED(hr)) return InitFailed();
            DXCall(frame.cmd_list->Close());

            NAME_D3D12_OBJECT_INDEXED(frame.cmd_allocator, i, L"Upload Command Allocator");
            NAME_D3D12_OBJECT_INDEXED(frame.cmd_list, i, L"Upload Command List");
        }

        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

        //create cmd queue
        DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&upload_cmd_queue)));
        if (FAILED(hr)) return InitFailed();
        NAME_D3D12_OBJECT(upload_cmd_queue, L"Upload Copy Queue");

        //create fence
        DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&upload_fence)));
        if (FAILED(hr)) return InitFailed();
        NAME_D3D12_OBJECT(upload_fence, L"Upload Fence");

        //create fence event
        fence_event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(fence_event);
        if (!fence_event) return InitFailed();

        return true;
    }

    void Shutdown()
    {
        for (u32 i{ 0 }; i < upload_frame_count; ++i)
        {
            upload_frames[i].Release();
        }

        if (fence_event)
        {
            CloseHandle(fence_event);
            fence_event = nullptr;
        }

        //core::release(upload_cmd_queue);
        if (upload_cmd_queue != nullptr) {
            upload_cmd_queue->Release();
            upload_cmd_queue = nullptr;
        }

        //core::release(upload_fence);
        if (upload_fence != nullptr) {
            upload_fence->Release();
            upload_fence = nullptr;
        }

        upload_fence_value = 0;
    }

}
