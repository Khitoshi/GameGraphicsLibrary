#pragma once
#include "../Components/ComponentsCommon.h"

namespace Transform
{
    DEFINE_TYPED_ID(TransformID);

    class Component
    {
    public:
        constexpr explicit Component(TransformID id) : id_{ id } {}
        constexpr Component() : id_{ Id::invalid_id } {}
        constexpr TransformID GetID() const { return id_; }
        constexpr bool IsValid() const { return Id::IsValid(id_); }

        Math::v4 Rotation() const;
        Math::v3 Orientation() const;
        Math::v3 Position() const;
        Math::v3 Scale() const;
    private:
        TransformID id_;
    };
};


