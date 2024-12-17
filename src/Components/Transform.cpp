#include "Transform.h"
#include "Entity.h"

namespace Transform
{
    namespace {

        Utility::vector<Math::v4> rotations;
        Utility::vector<Math::v3> orientations;
        Utility::vector<Math::v3> positions;
        Utility::vector<Math::v3> scales;

        Math::v3 CalculateOrientation(Math::v4 rotation)
        {
            using namespace DirectX;
            XMVECTOR rotation_quat{ XMLoadFloat4(&rotation) };
            XMVECTOR front{ XMVectorSet(0.f, 0.f, 1.f, 0.f) };
            Math::v3 orientation;
            XMStoreFloat3(&orientation, XMVector3Rotate(front, rotation_quat));
            return orientation;
        }
    } // anonymous namespace

    Component Create(InitInfo info, GameEntity::Entity entity)
    {
        assert(entity.IsValid());
        const Id::id_type entity_index{ Id::Index(entity.GetID()) };

        if (positions.size() > entity_index) {
            Math::v4 rotation{ info.rotation };
            rotations[entity_index] = rotation;
            orientations[entity_index] = CalculateOrientation(rotation);
            positions[entity_index] = Math::v3{ info.position };
            scales[entity_index] = Math::v3{ info.scale };
        }
        else {
            assert(positions.size() == entity_index);
            rotations.emplace_back(info.rotation);
            orientations.emplace_back(CalculateOrientation(Math::v4{ info.rotation }));
            positions.emplace_back(info.position);
            scales.emplace_back(info.scale);
        }

        return Component{ TransformID{ entity.GetID() } };
    }

    void Remove([[maybe_unused]] Component c)
    {
        assert(c.IsValid());
    }

#pragma region Getter
    Math::v4 Component::Rotation() const
    {
        assert(IsValid());
        return rotations[Id::Index(id_)];
    }

    Math::v3 Component::Orientation() const
    {
        assert(IsValid());
        return orientations[Id::Index(id_)];
    }

    Math::v3 Component::Position() const
    {
        assert(IsValid());
        return positions[Id::Index(id_)];
    }

    Math::v3 Component::Scale() const
    {
        assert(IsValid());
        return scales[Id::Index(id_)];
    }
#pragma endregion// Getter
}

