#pragma once
#include "CommonHeaders.h"

namespace Id
{
    using id_type = u32;

    namespace Detail {
        constexpr u32 generation_bits{ 10 };
        constexpr u32 index_bits{ sizeof(id_type) * 8 - generation_bits };
        constexpr id_type index_mask{ (id_type{1} << index_bits) - 1 };
        constexpr id_type generation_mask{ (id_type{1} << generation_bits) - 1 };
    } // Detail namespace


    constexpr u32 min_deleted_elements{ 1024 };

    using generation_type = std::conditional_t<Detail::generation_bits <= 16, std::conditional_t<Detail::generation_bits <= 8, u8, u16>, u32>;
    static_assert(sizeof(generation_type) * 8 >= Detail::generation_bits);
    static_assert((sizeof(id_type) - sizeof(generation_type)) > 0);

    constexpr id_type invalid_id{ id_type(-1) };
    constexpr bool IsValid(id_type id)
    {
        return id != invalid_id;
    }

    constexpr id_type Index(id_type id)
    {
        id_type index{ id & Detail::index_mask };
        assert(index != Detail::index_mask);
        return index;
    }

    constexpr id_type Generation(id_type id)
    {
        return (id >> Detail::index_bits) & Detail::generation_mask;
    }

    constexpr id_type NewGeneration(id_type id)
    {
        const id_type generation{ Id::Generation(id) + 1 };
        assert(generation < (((u64)1 << Detail::generation_bits) - 1));
        return Index(id) | (generation << Detail::index_bits);
    }

#if _DEBUG
    namespace Detail {
        struct IdBase
        {
            constexpr explicit IdBase(id_type id) : _id{ id } {}
            constexpr operator id_type() const { return _id; }
        private:
            id_type _id;
        };
    } // Detail namespace

#define DEFINE_TYPED_ID(name)                       \
        struct name final : Id::Detail::IdBase      \
        {                                           \
            constexpr explicit name(Id::id_type id) \
                : IdBase{ id } {}                   \
            constexpr name() : IdBase{ 0 } {}       \
        };
#else
#define DEFINE_TYPED_ID(name) using name = Id::id_type;
#endif
} // Id namespace
