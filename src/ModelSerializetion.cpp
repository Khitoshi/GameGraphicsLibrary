#include "ModelSerializetion.h"
#include "Renderer/Renderer.h"
#include "Content/FbxImporter.h"
#include "Content/Geometry.h" 

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace
{

    class ModelSerializer
    {
    public:
        ModelSerializer() = default;
        ~ModelSerializer() = default;


    private:

    };

    struct ModelFileInfo
    {
        const char* file;
    };

    constexpr ModelFileInfo model_files[]
    {
        {"aaa.fbx"},
    };

    constexpr const char* models_data_path{ "../../Assets/Models" };

    // シリアライズされたモデルの最終更新日時をチェック
    bool SerializedModelsAreUpToDate()
    {
        auto engine_models_path = Renderer::GetEngineModelsPath();
        if (!std::filesystem::exists(engine_models_path)) return false;

        //シリアライズされたモデルの最終更新日時
        const auto models_serializetion_time{ std::filesystem::last_write_time(engine_models_path) };

        for (const auto& entry : fs::directory_iterator(models_data_path))
        {
            //TODO 変更する
            if (entry.is_regular_file() && entry.path().extension() != ".fbx") continue;
            const auto model_file_time = std::filesystem::last_write_time(entry.path());
            if (model_file_time > models_serializetion_time)return false;
        }

        return true;
    }
}

bool ModelSerializetion()
{
    if (SerializedModelsAreUpToDate())return true;

    //Utility::vector<> models;
    std::filesystem::path path{};
    std::filesystem::path full_path{};

    for (auto& info : model_files)
    {
        // ソースファイルの最終更新日時とコンパイル日時を比較
        path = models_data_path;
        path += info.file;
        full_path = path;
        if (!std::filesystem::exists(full_path)) return false;





        // モデルのシリアライズ
        Content::Scene scene;
        Content::SceneData scene_data;
        Progression progression;
        Content::FBXContext fbx_context{ full_path.string().c_str(),&scene, &scene_data, &progression };



    }

    return true;
}
