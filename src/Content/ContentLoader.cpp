#include "ContentLoader.h"
#include "../Renderer/Renderer.h"

#include <fstream>
#include <filesystem>
#include <Windows.h>

namespace Content
{
    namespace
    {
        bool ReadFile(std::filesystem::path path, std::unique_ptr<u8[]>& data, u64& size)
        {
            if (!std::filesystem::exists(path)) return false;

            size = std::filesystem::file_size(path);
            assert(size);
            if (!size) return false;
            data = std::make_unique<u8[]>(size);
            std::ifstream file{ path, std::ios::in | std::ios::binary };
            if (!file || !file.read((char*)data.get(), size))
            {
                file.close();
                return false;
            }

            file.close();
            return true;
        }

    }

    bool LoadEngineShaders(std::unique_ptr<u8[]>& shaders, u64& size)
    {
        auto path = Renderer::GetEngineShadersPath();
        return ReadFile(path, shaders, size);
    }

}