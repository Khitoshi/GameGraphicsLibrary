#include "Entity.h"
#include "Transform.h"

namespace GameEntity {

    namespace {

        Utility::vector<Transform::Component>   transforms;
        //Utility::vector<script::component>          scripts;

        Utility::vector<Id::generation_type>    generations;
        Utility::deque<EntityID>                free_ids;
    } // anonymous namespace


    Entity Create(EntityInfo info)
    {
        assert(info.transform); // All game entities must have a transform component
        if (!info.transform) return Entity{};

        EntityID id;

        if (free_ids.size() > Id::min_deleted_elements) {
            id = free_ids.front();
            assert(!IsAlive(id));
            free_ids.pop_front();
            id = EntityID{ Id::NewGeneration(id) };
            ++generations[Id::Index(id)];
        }
        else {
            id = EntityID{ (Id::id_type)generations.size() };
            generations.push_back(0);

            // Resize components
            // NOTE: we don't call resize(), so the number of memory allocations stays low
            transforms.emplace_back();
            //script.emplace_back();
        }

        const Entity new_entity{ id };
        const Id::id_type index{ Id::Index(id) };

        // Create transform component
        assert(!transforms[index].IsValid());
        transforms[index] = Transform::Create(*info.transform, new_entity);
        if (!transforms[index].IsValid()) return {};

        // Create script component
        //if (info.script && info.script->script_creator)
        //{
        //    assert(!scripts[index].IsValid());
        //    scripts[index] = script::create(*info.script, new_entity);
        //    assert(scripts[index].is_valid());
        //}

        return new_entity;
    }

    void Remove(EntityID id)
    {
        const Id::id_type index{ Id::Index(id) };
        assert(IsAlive(id));

        //if (scripts[index].is_valid())
        //{
        //    script::remove(scripts[index]);
        //    scripts[index] = {};
        //}

        Transform::Remove(transforms[index]);
        transforms[index] = {};
        free_ids.push_back(id);
    }

    bool IsAlive(EntityID id)
    {
        assert(Id::IsValid(id));
        const Id::id_type index{ Id::Index(id) };
        assert(index < generations.size());
        return (generations[index] == Id::Generation(id) && transforms[index].IsValid());
    }

    Transform::Component Entity::Transform() const
    {
        assert(IsAlive(id_));
        const Id::id_type index{ Id::Index(id_) };
        return transforms[index];
    }

}

