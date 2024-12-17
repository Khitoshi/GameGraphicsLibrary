#pragma once

#include "D3D12CommonHeaders.h"
#include <PrimitiveTypes.h>

namespace RHI::D3D12::CONTENT {
    namespace SUBMESH {

        Id::id_type Add(const u8*& data);
        void Remove(Id::id_type id);

    } // namespace submesh
}
