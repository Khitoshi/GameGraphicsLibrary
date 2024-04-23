#include "D3D12Shaders.h"
#include "../Content/ContentLoader.h"
namespace RHI::D3D12::Shaders
{

    namespace {
        typedef struct CompiledShader
        {
            u64         size;
            const u8* byte_code;
        } const* compiled_shader_ptr;

        // この配列の各要素は、shader.blob内のオフセットを指す
        compiled_shader_ptr engine_shaders[EngineShader::COUNT]{};

        // コンパイルされたすべてのエンジンシェーダーを含むメモリの塊(u64サイズとバイト配列からなるシェーダーバイトコードの配列) 
        std::unique_ptr<u8[]> shaders_blob{};

        bool LoadEngineShaders()
        {
            assert(!shaders_blob);
            u64 size{ 0 };
            bool result{ Content::LoadEngineShaders(shaders_blob, size) };
            assert(shaders_blob && size);

            u64 offset{ 0 };
            u32 index{ 0 };
            while (offset < size && result)
            {
                assert(index < EngineShader::COUNT);
                compiled_shader_ptr& shader{ engine_shaders[index] };
                assert(!shader);
                result &= index < EngineShader::COUNT && !shader;
                if (!result) break;
                shader = reinterpret_cast<const compiled_shader_ptr>(&shaders_blob[offset]);
                offset += sizeof(u64) + shader->size;
                ++index;
            }
            assert(offset == size && index == EngineShader::COUNT);

            return result;
        }
    }

    bool Initialize()
    {
        return LoadEngineShaders();
    }

    void Shutdown()
    {
        for (u32 i{ 0 }; i < EngineShader::COUNT; ++i)
        {
            engine_shaders[i] = {};
        }
        shaders_blob.reset();
    }

#pragma region Getter
    D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id)
    {
        assert(id < EngineShader::COUNT);
        const compiled_shader_ptr shader{ engine_shaders[id] };
        assert(shader && shader->size);
        return { &shader->byte_code, shader->size };
    }
#pragma endregion// Getter

}

