#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Shaders {
#undef DOMAIN // Windows.hÇÃÉ}ÉNÉçDOMAINÇ∆è’ìÀÇ∑ÇÈÇÃÇ≈undefÇ∑ÇÈ
    struct ShaderType {
        enum TYPE : u32 {
            VERTEX = 0,
            HULL,
            DOMAIN,
            GEOMETRY,
            PIXEL,
            COMPUTE,
            AMPLIFICATION,
            MESH,

            COUNT
        };
    };

    struct EngineShader {
        enum ID : u32 {
            FULLSCREENTRIANGLE_VS = 0,
            FILLCOLOR_PS = 1,

            COUNT
        };
    };

    bool Initialize();
    void Shutdown();

#pragma region Getter
    D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
#pragma endregion //Getter

}