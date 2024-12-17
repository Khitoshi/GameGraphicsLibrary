#pragma once

#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::UPLOAD {

    class D3D12UploadContext {
    public:
        D3D12UploadContext(u32 aligned_size);
        DISABLE_COPY_AND_MOVE(D3D12UploadContext);
        ~D3D12UploadContext() = default;

        void EndUpload();

#pragma region Gettter
        constexpr ID3D12GraphicsCommandList6* const CommandList() const { return cmd_list_; }
        constexpr ID3D12Resource* const UploadBuffer() const { return upload_buffer_; }
        constexpr void* const CpuAddress() const { return cpu_address_; }
#pragma endregion// Getter
    private:
        DEBUG_OP(D3D12UploadContext() = default);
        ID3D12GraphicsCommandList6* cmd_list_{ nullptr };
        ID3D12Resource* upload_buffer_{ nullptr };
        void* cpu_address_{ nullptr };
        u32 frame_index_{ u32_invalid_id };
    };

    bool Initialize();
    void Shutdown();
}