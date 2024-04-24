#pragma once

#include "D3D12Core.h"
#include "D3D12Helpers.h"

namespace RHI::D3D12::Parts {
    ID3D12RootSignature* root_signature_;
    D3D12_SHADER_BYTECODE vs_{};

    template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
    class alignas(void*) D3D12PipelineStateSubobject
    {
    public:
        D3D12PipelineStateSubobject() = default;
        constexpr explicit D3D12PipelineStateSubobject(T subobject) : type_{ type }, subobject_{ subobject } {}
        D3D12PipelineStateSubobject& operator=(const T& subobject) { subobject_ = subobject; return *this; }
    private:
        const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type_{ type };
        T subobject_{};
    };

    using D3D12PipelineStateSubobject_RootSignature = D3D12PipelineStateSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*>;
    using D3D12PipelineStateSubobject_VS = D3D12PipelineStateSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE>;

}