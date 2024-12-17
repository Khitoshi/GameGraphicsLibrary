#include "ShaderCompilation.h"

#include "..\packages\DirectXShaderCompiler\inc\d3d12shader.h"
#include "..\packages\DirectXShaderCompiler\inc\dxcapi.h"

#include "./RHI/DirectX12/D3D12Core.h"
#include "./RHI/DirectX12/D3D12Shaders.h"
#include "./Renderer/Renderer.h"

#include <fstream>
#include <filesystem>

//#pragma comment(lib, "../../packages/DirectXShaderCompiler/lib/x64/dxcompiler.lib")

using namespace RHI::D3D12::Shaders;
using namespace Microsoft::WRL;

namespace {

    struct ShaderFileInfo
    {
        const char* file;
        const char* function;
        EngineShader::ID    id;
        ShaderType::TYPE    type;
    };

    constexpr ShaderFileInfo shader_files[]
    {
        {"FullScreenTriangle.hlsl", "FullScreenTriangleVS", EngineShader::FULLSCREENTRIANGLE_VS,    ShaderType::VERTEX},
        {"FillColor.hlsl",          "FillColorPS",          EngineShader::FILLCOLOR_PS,             ShaderType::PIXEL},
        {"PostProcess.hlsl",        "PostProcessPS",        EngineShader::POSTPROCESS_PS,           ShaderType::PIXEL},
    };
    static_assert(_countof(shader_files) == EngineShader::COUNT);

    constexpr const char* shaders_source_path{ "../../src/RHI/DirectX12/Shaders/" };

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

            DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_)));
            if (FAILED(hr)) return;

            DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils_)));
            if (FAILED(hr)) return;

            DXCall(hr = utils_->CreateDefaultIncludeHandler(&include_handler_));
            if (FAILED(hr)) return;
        }

        DISABLE_COPY_AND_MOVE(ShaderCompiler);

        IDxcBlob* Compile(ShaderFileInfo info, std::filesystem::path full_path)
        {
            assert(compiler_ && utils_ && include_handler_);
            HRESULT hr{ S_OK };

            // ソースファイルをロード
            ComPtr<IDxcBlobEncoding> source_blob{ nullptr };
            DXCall(hr = utils_->LoadFile(full_path.c_str(), nullptr, &source_blob));
            if (FAILED(hr)) return nullptr;
            assert(source_blob && source_blob->GetBufferSize());

            std::wstring file{ ToWstring(info.file) };
            std::wstring func{ ToWstring(info.function) };
            std::wstring prof{ ToWstring(profile_strings_[(u32)info.type]) };
            std::wstring inc{ ToWstring(shaders_source_path) };

            // コンパイルオプション
            LPCWSTR args[]
            {
                file.c_str(),                       // エラー報告用のオプションのシェーダー・ソース・ファイル名
                L"-E", func.c_str(),                // エントリー機能
                L"-T", prof.c_str(),                // ターゲット・プロフィール
                L"-I", inc.c_str(),
                DXC_ARG_ALL_RESOURCES_BOUND,
    #if _DEBUG
                DXC_ARG_DEBUG,                      // デバッグ情報を含む
                DXC_ARG_SKIP_OPTIMIZATIONS,         // 最適化をスキップ
    #else
                DXC_ARG_OPTIMIZATION_LEVEL3,        // 最適化レベル3
    #endif
                DXC_ARG_WARNINGS_ARE_ERRORS,        // WARNINGをERRORとして扱う
                L"-Qstrip_reflect",                 // reflectを別の塊に分離する
                L"-Qstrip_debug",                   // デバッグ情報を別のblobに取り出す
            };

            OutputDebugStringA("Compiling ");
            OutputDebugStringA(info.file);

            return Compile(source_blob.Get(), args, _countof(args));
        }

        IDxcBlob* Compile(IDxcBlobEncoding* source_blob, LPCWSTR* args, u32 num_args)
        {
            DxcBuffer buffer{};
            buffer.Encoding = DXC_CP_ACP;                   // テキスト形式の自動検出
            buffer.Ptr = source_blob->GetBufferPointer();
            buffer.Size = source_blob->GetBufferSize();

            // コンパイル実行
            HRESULT hr{ S_OK };
            ComPtr<IDxcResult> results{ nullptr };
            DXCall(hr = compiler_->Compile(&buffer, args, num_args, include_handler_.Get(), IID_PPV_ARGS(&results)));
            if (FAILED(hr)) return nullptr;

            {// エラー検出&表示
                ComPtr<IDxcBlobUtf8> errors{ nullptr };
                DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
                if (FAILED(hr)) return nullptr;

                if (errors && errors->GetStringLength()) {
                    OutputDebugStringA("\nShader compilation error: \n");
                    OutputDebugStringA(errors->GetStringPointer());
                }
                else {
                    OutputDebugStringA(" [ Succeeded ]");
                }
                OutputDebugStringA("\n");
            }

            // コンパイル結果の確認
            HRESULT status{ S_OK };
            DXCall(hr = results->GetStatus(&status));
            if (FAILED(hr) || FAILED(status)) return nullptr;

            // コンパイル結果の取得
            ComPtr<IDxcBlob> shader{ nullptr };
            DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
            if (FAILED(hr)) return nullptr;

            // shaderをデタッチして所有権を返す
            return shader.Detach();
        }

    private:

        constexpr static const char* profile_strings_[ShaderType::COUNT]
        {
            "vs_6_5",
            "hs_6_5",
            "ds_6_5",
            "gs_6_5",
            "ps_6_5",
            "cs_6_5",
            "as_6_5",
            "ms_6_5"
        };
        static_assert(_countof(profile_strings_) == ShaderType::COUNT);

        ComPtr<IDxcCompiler3>       compiler_{ nullptr };
        ComPtr<IDxcUtils>           utils_{ nullptr };
        ComPtr<IDxcIncludeHandler>  include_handler_{ nullptr };
    };

    // コンパイルされたシェーダーのバイナリファイルへのパスを取得
    //decltype(auto) は、戻り値の型をコンパイラを推測させ、使用する
    decltype(auto) GetEngineShadersPath()
    {
        return std::filesystem::path{ Renderer::GetEngineShadersPath(Renderer::GRAPHICS_PLATFORM::direct3d12) };
    }

    // コンパイルされたシェーダーが最新であるかどうかをチェック
    bool CompiledShadersAreUpToDate()
    {
        auto engine_shaders_path = GetEngineShadersPath();
        if (!std::filesystem::exists(engine_shaders_path)) return false;

        //シェーダーの最終コンパイル日時
        const auto shaders_compilation_time{ std::filesystem::last_write_time(engine_shaders_path) };

        std::filesystem::path path{};
        std::filesystem::path full_path{};

        // ソースファイルがシェーダーの最終コンパイル日時より新しくなっていないかチェック
        //for (auto& info: shader_files)としていないのは、EngineShaderの数と相違していないかチェックを行いたいため
        for (u32 i{ 0 }; i < EngineShader::COUNT; ++i)
        {
            auto& info = shader_files[i];
            // ソースファイルの最終更新日時とコンパイル日時を比較
            path = shaders_source_path;
            path += info.file;
            full_path = path;
            if (!std::filesystem::exists(full_path)) return false;
            auto shader_file_time = std::filesystem::last_write_time(full_path);
            if (shader_file_time > shaders_compilation_time)return false;
        }

        return true;
    }

    // コンパイルされたシェーダーをバイナリファイルに保存
    bool SaveCompiledShaders(Utility::vector<ComPtr<IDxcBlob>>& shaders)
    {
        //バイナリファイルを保存するディレクトリ & バイナリファイル が存在しない場合は作成
        auto engine_shaders_path = GetEngineShadersPath();
        std::filesystem::create_directories(engine_shaders_path.parent_path());
        std::ofstream file(engine_shaders_path, std::ios::out | std::ios::binary);
        if (!file || !std::filesystem::exists(engine_shaders_path)) {
            file.close();
            return false;
        }

        // バイナリファイルにシェーダー情報の書き込み
        for (auto& shader : shaders)
        {
            const D3D12_SHADER_BYTECODE byte_code{ shader->GetBufferPointer(), shader->GetBufferSize() };
            file.write((char*)&byte_code.BytecodeLength, sizeof(byte_code.BytecodeLength));
            file.write((char*)byte_code.pShaderBytecode, byte_code.BytecodeLength);
        }

        file.close();
        return true;
    }

} // anonymous namespace

bool CompileShaders()
{
    // コンパイルされたシェーダーが最新であるかどうかをチェックし、最新であればコンパイルをスキップ
    if (CompiledShadersAreUpToDate()) return true;

    Utility::vector<ComPtr<IDxcBlob>> shaders;
    std::filesystem::path path{};
    std::filesystem::path full_path{};

    ShaderCompiler compiler{};
    // シェーダーをコンパイルし、それらを同じコンパイル順序でバッファにまとめる
    for (u32 i{ 0 }; i < EngineShader::COUNT; ++i)
    {
        auto& info = shader_files[i];
        path = shaders_source_path;
        path += info.file;
        full_path = path;
        if (!std::filesystem::exists(full_path)) return false;

        // シェーダーをコンパイル
        ComPtr<IDxcBlob> compiled_shader{ compiler.Compile(info, full_path) };
        if (compiled_shader && compiled_shader->GetBufferPointer() && compiled_shader->GetBufferSize())shaders.emplace_back(std::move(compiled_shader));
        else return false;
    }

    return SaveCompiledShaders(shaders);
}