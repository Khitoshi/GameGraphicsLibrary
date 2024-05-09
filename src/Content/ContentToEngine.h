#pragma once
#include "CommonHeaders.h"

namespace CONTENT {
    struct PrimitveTopology {
        enum TYPE : u32 {
            POINT_LIST = 1,
            LINE_LIST,
            LINE_STRIP,
            TRIANGLE_LIST,
            TRIANGLE_STRIP,

            COUNT
        };
    };

}
