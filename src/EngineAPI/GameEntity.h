#pragma once
#include "../Components/ComponentsCommon.h"
#include "TransfromComponent.h"

namespace GameEntity {

    DEFINE_TYPED_ID(EntityID);

    class Entity {
    public:
        constexpr explicit  Entity(EntityID id) : id_{ id } {}
        constexpr           Entity() : id_{ Id::invalid_id } {}
        constexpr EntityID  GetID() const { return id_; }
        constexpr bool      IsValid() const { return Id::IsValid(id_); }

        Transform::Component Transform() const;
        //script::Component script() const;
    private:
        EntityID id_;
    };
} // namespace GameEntity

/*
namespace script
{
    class entity_script : public game_entity::entity
    {
    public:
        virtual ~entity_script() = default;
        virtual void begin_play() {}
        virtual void update(float) {}
    protected:
        constexpr explicit entity_script(game_entity::entity entity)
            : game_entity::entity{ entity.get_id() } {}
    };

    namespace detail {
        using script_ptr = std::unique_ptr<entity_script>;
        using script_creator = script_ptr(*)(game_entity::entity entity);
        using string_hash = std::hash<std::string>;

        u8 register_script(size_t, script_creator);
#ifdef USE_WITH_EDITOR
        extern "C" __declspec(dllexport)
#endif //USE_WITH_EDITOR
            script_creator get_script_creator(size_t tag);

        template<class script_class>
        script_ptr create_script(game_entity::entity entity)
        {
            assert(entity.is_valid());
            return std::make_unique<script_class>(entity);
        }

#ifdef USE_WITH_EDITOR
        u8 add_script_name(const char* name);

#define REGISTER_SCRIPT(TYPE)                                           \
        namespace {                                                     \
        const u8 _reg_##TYPE                                            \
        { primal::script::detail::register_script(                      \
              primal::script::detail::string_hash()(#TYPE),             \
              &primal::script::detail::create_script<TYPE>) };          \
        const u8 _name_##TYPE                                           \
        { primal::script::detail::add_script_name(#TYPE) };             \
        }

#else
#define REGISTER_SCRIPT(TYPE)                                           \
        namespace {                                                     \
        const u8 _reg_##TYPE                                            \
        { primal::script::detail::register_script(                      \
              primal::script::detail::string_hash()(#TYPE),             \
              &primal::script::detail::create_script<TYPE>) };          \
        }

#endif // USE_WITH_EDITOR
    } // namespace detail
} // namespace script
*/