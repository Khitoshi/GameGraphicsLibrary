#pragma once

#include "ComponentsCommon.h"

#define INIT_INFO(Component) namespace Component { struct InitInfo; }

INIT_INFO(Transform);
//INIT_INFO(Script);

#undef INIT_INFO

namespace GameEntity {
    struct EntityInfo
    {
        Transform::InitInfo* transform{ nullptr };
        //Script::init_info* script{ nullptr };
    };

    Entity  Create(EntityInfo info);
    void    Remove(EntityID id);
    bool    IsAlive(EntityID id);
}