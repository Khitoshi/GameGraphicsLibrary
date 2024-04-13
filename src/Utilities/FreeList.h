#pragma once
#include "CommonHeaders.h"

namespace Utility {

#if USE_STL_VECTOR
#pragma message("WARNING: using utl::free_list with std::vector result in duplicate calls to class constructor!")
#endif

    template<typename T>
    class free_list
    {
        static_assert(sizeof(T) >= sizeof(u32));
    public:
        free_list() = default;
        explicit free_list(u32 count)
        {
            array_.reserve(count);
        }

        ~free_list()
        {
            assert(!size_);
#if USE_STL_VECTOR
            memset(array_.data(), 0, array_.size() * sizeof(T));
#endif
        }

        template<class... params>
        constexpr u32 add(params&&... p)
        {
            u32 id{ u32_invalid_id };
            if (next_free_index_ == u32_invalid_id) {
                id = (u32)array_.size();
                array_.emplace_back(std::forward<params>(p)...);
            }
            else {
                id = next_free_index_;
                assert(id < array_.size() && already_removed(id));
                next_free_index_ = *(const u32* const)std::addressof(array_[id]);
                new (std::addressof(array_[id])) T(std::forward<params>(p)...);
            }
            ++size_;
            return id;
        }

        constexpr void remove(u32 id)
        {
            assert(id < array_.size() && !already_removed(id));
            T& item{ array_[id] };
            item.~T();
            DEBUG_OP(memset(std::addressof(array_[id]), 0xcc, sizeof(T)));
            *(u32* const)std::addressof(array_[id]) = next_free_index_;
            next_free_index_ = id;
            --size_;
        }

        constexpr u32 size() const
        {
            return size_;
        }

        constexpr u32 capacity() const
        {
            return array_.size();
        }

        constexpr bool empty() const
        {
            return size_ == 0;
        }

        [[nodiscard]] constexpr T& operator[](u32 id)
        {
            assert(id < array_.size() && !already_removed(id));
            return array_[id];
        }

        [[nodiscard]] constexpr const T& operator[](u32 id) const
        {
            assert(id < array_.size() && !already_removed(id));
            return array_[id];
        }

    private:
        constexpr bool already_removed(u32 id)
        {
            // NOTE: sizeof(T)==sizeof(u32)の場合、アイテムがすでに削除されているかどうかをテストすることはできない
            if constexpr (sizeof(T) > sizeof(u32))
            {
                u32 i{ sizeof(u32) }; //skip the first 4 bytes.
                const u8* const p{ (const u8* const)std::addressof(array_[id]) };
                while ((p[i] == 0xcc) && (i < sizeof(T))) ++i;
                return i == sizeof(T);
            }
            else
            {
                return true;
            }
        }
#if USE_STL_VECTOR
        Utility::vector<T>          array_;
#else
        Utility::vector<T, false>   array_;
#endif
        u32                     next_free_index_{ u32_invalid_id };
        u32                     size_{ 0 };
    };
}