#pragma once
#include "CommonHeaders.h"

#if !defined(SHIPPING)
namespace Content {
    //bool LoadGame();
    //void UnloadGame();

    bool LoadEngineShaders(std::unique_ptr<u8[]>& shaders, u64& size);
}
#endif // !defined(SHIPPING)