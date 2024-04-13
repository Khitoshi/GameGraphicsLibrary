#pragma once
#include "CommonHeaders.h"

namespace Utility {

    // std::vectorに似たベクター・クラスで、基本的な機能を備えています。 
    // ユーザーはテンプレート引数で、要素のデストラクタを、削除されるときに呼び出すか、
    // ベクターをクリア/破壊するときに呼び出すかを指定できる。
    template<typename T, bool destruct = true>
    class vector
    {
    public:
        // デフォルトのコンストラクタ。メモリを割り当てない。
        vector() = default;

        // コンストラクタはベクトルのサイズを変更し、'count' アイテムを初期化
        constexpr explicit vector(u64 count)
        {
            resize(count);
        }

        // コンストラクタはベクトルのサイズを変更し、'value'を使って'count'項目を初期化します。
        constexpr explicit vector(u64 count, const T& value)
        {
            resize(count, value);
        }

        template<typename it, typename = std::enable_if_t<std::_Is_iterator_v<it>>>
        constexpr explicit vector(it first, it last)
        {
            for (; first != last; ++first)
            {
                emplace_back(*first);
            }
        }

        //コピー・コンストラクタ。別のベクトルをコピーして構築する。
        // コピーされたベクトル内の項目はコピー可能でなければならない
        constexpr vector(const vector& o)
        {
            *this = o;
        }

        // 移動コンストラクタ。別のベクトルを移動して構築します。 
        // 移動後の元のベクトルは空になる
        constexpr vector(const vector&& o)
            : capacity_{ o.capacity_ }, size_{ o.size_ }, data_{ o.data_ }
        {
            o.reset();
        }

        // コピー代入演算子。このベクトルをクリアし、別のベクトルから項目をコピーします。
        // 項目はコピー可能でなければならない
        constexpr vector& operator=(const vector& o)
        {
            assert(this != std::addressof(o));
            if (this != std::addressof(o))
            {
                clear();
                reserve(o.size_);
                for (auto& item : o)
                {
                    emplace_back(item);
                }
                assert(size_ == o.size_);
            }

            return *this;
        }

        // 移動代入演算子
        // このベクトル内のすべてのリソースを解放し、もう一方のベクトルをこのベクトルに移動する
        constexpr vector& operator=(vector&& o)
        {
            assert(this != std::addressof(o));
            if (this != std::addressof(o))
            {
                destroy();
                move(o);
            }

            return *this;
        }

        // テンプレート引数で指定されたベクトルとそのアイテムを破壊します
        ~vector() { destroy(); }

        // value'をコピーして、ベクターの末尾に項目を挿入する
        constexpr void push_back(const T& value)
        {
            emplace_back(value);
        }

        // value'を移動させて、ベクターの末尾に項目を挿入する
        constexpr void push_back(T&& value)
        {
            emplace_back(std::move(value));
        }

        // ベクターの末尾に項目をコピーまたは移動する
        template<typename... params>
        constexpr decltype(auto) emplace_back(params&&... p)
        {
            if (size_ == capacity_) {
                reserve(((capacity_ + 1) * 3) >> 1); // reserve 50% more
            }
            assert(size_ < capacity_);

            T* const item{ new (std::addressof(data_[size_])) T(std::forward<params>(p)...) };
            ++size_;
            return item;
        }

        // ベクトルのサイズを変更し、新しい項目をデフォルト値で初期化します
        constexpr void resize(u64 new_size)
        {
            static_assert(std::is_default_constructible_v<T>,
                "Type must be default-constructible.");

            if (new_size > size_) {
                reserve(new_size);
                while (size_ < new_size) {
                    emplace_back();
                }
            }
            else if (new_size < size_) {
                if constexpr (destruct) {
                    destruct_range(new_size, size_);
                }
                size_ = new_size;
            }

            // Do nothing if newsize_ == size_.
            assert(new_size == size_);
        }

        // ベクトルのサイズを変更し、'value'をコピーして新しい項目を初期化する
        constexpr void resize(u64 new_size, const T& value)
        {
            static_assert(std::is_copy_constructible_v<T>,
                "Type must be copy-constructible.");

            if (new_size > size_) {
                reserve(new_size);
                while (size_ < new_size) {
                    emplace_back(value);
                }
            }
            else if (new_size < size_) {
                if constexpr (destruct) {
                    destruct_range(new_size, size_);
                }
                size_ = new_size;
            }

            // Do nothing if newsize_ == size_.
            assert(new_size == size_);
        }

        // 指定された数のアイテムを格納するメモリを確保する
        constexpr void reserve(u64 newcapacity_)
        {
            if (newcapacity_ > capacity_) {
                // NOTE: realoc()は、新しいメモリ領域が割り当てられると、バッファ内のデータを自動的にコピーする
                void* new_buffer{ realloc(data_, newcapacity_ * sizeof(T)) };
                assert(new_buffer);
                if (new_buffer) {
                    data_ = static_cast<T*>(new_buffer);
                    capacity_ = newcapacity_;
                }
            }
        }

        // 指定されたインデックスの項目を削除する  
        constexpr T* const erase(u64 index)
        {
            assert(data_ && index < size_);
            return erase(std::addressof(data_[index]));
        }

        // 指定した位置のアイテムを削除します
        constexpr T* const erase(T* const item)
        {
            assert(data_ && item >= std::addressof(data_[0]) &&
                item < std::addressof(data_[size_]));
            if constexpr (destruct) item->~T();
            --size_;
            if (item < std::addressof(data_[size_])) {
                memcpy(item, item + 1, (std::addressof(data_[size_]) - item) * sizeof(T));
            }

            return item;
        }

        // erase()と同じだが、最後の項目をコピーするだけなので高速。
        constexpr T* const erase_unordered(u64 index)
        {
            assert(data_ && index < size_);
            return erase_unordered(std::addressof(data_[index]));
        }

        // erase()と同じだが、最後の項目をコピーするだけなので高速。
        constexpr T* const erase_unordered(T* const item)
        {
            assert(data_ && item >= std::addressof(data_[0]) &&
                item < std::addressof(data_[size_]));
            if constexpr (destruct) item->~T();
            --size_;
            if (item < std::addressof(data_[size_])) {
                memcpy(item, std::addressof(data_[size_]), sizeof(T));
            }

            return item;
        }

        // ベクターをクリアし、テンプレート引数で指定されたアイテムを破棄します。
        constexpr void clear()
        {
            if constexpr (destruct) {
                destruct_range(0, size_);
            }
            size_ = 0;
        }

        // 2つのベクトルを入れ替える
        constexpr void swap(vector& o)
        {
            if (this != std::addressof(o)) {
                auto temp(std::move(o));
                o.move(*this);
                move(temp);
            }
        }

        // データの開始位置へのポインタ。NULLかもしれない。
        [[nodiscard]] constexpr T* data()
        {
            return data_;
        }

        // データの開始位置へのポインタ。NULLかもしれない。
        [[nodiscard]] constexpr T* const data() const
        {
            return data_;
        }

        // vector が空の場合に真を返す。
        [[nodiscard]] constexpr bool empty() const
        {
            return size_ == 0;
        }

        // ベクトル内のアイテムの数を返します。
        [[nodiscard]] constexpr u64 size() const
        {
            return size_;
        }

        // ベクトルの現在の容量を返します。
        [[nodiscard]] constexpr u64 capacity() const
        {
            return capacity_;
        }

        // インデックス演算子。指定したインデックスの項目への参照を返します。
        [[nodiscard]] constexpr T& operator[](u64 index)
        {
            assert(data_ && index < size_);
            return data_[index];
        }


        // インデックス演算子。指定されたインデックスの項目への定数参照を返します。
        [[nodiscard]] constexpr const T& operator[](u64 index) const
        {
            assert(data_ && index < size_);
            return data_[index];
        }

        // 最初の項目への参照を返す 
        // ベクターが空のときに呼び出されると、アプリケーションはエラーになります。
        [[nodiscard]] constexpr T& front()
        {
            assert(data_ && size_);
            return data_[0];
        }

        // 最初の項目への定数参照を返す
        // ベクターが空のときに呼び出されると、アプリケーションはエラーになります。
        [[nodiscard]] constexpr const T& front() const
        {
            assert(data_ && size_);
            return data_[0];
        }

        // 最後の項目への参照を返返す
        // ベクターが空のときに呼び出されると、アプリケーションはエラーになります。
        [[nodiscard]] constexpr T& back()
        {
            assert(data_ && size_);
            return data_[size_ - 1];
        }

        // 最後の項目への定数参照を返す
        // ベクターが空のときに呼び出されると、アプリケーションはエラーになります。
        [[nodiscard]] constexpr const T& back() const
        {
            assert(data_ && size_);
            return data_[size_ - 1];
        }

        // 最初の項目へのポインタを返します。
        // vector が空の場合は null を返す。
        [[nodiscard]] constexpr T* begin()
        {
            assert(data_);
            return std::addressof(data_[0]);
        }

        // 最初の項目への定数ポインタを返します。
        // vector が空の場合は null を返す。
        [[nodiscard]] constexpr const T* begin() const
        {
            assert(data_);
            return std::addressof(data_[0]);
        }

        // 最後の項目へのポインタを返します。vector が空の場合は null を返します。
        [[nodiscard]] constexpr T* end()
        {
            assert(data_);
            return std::addressof(data_[size_]);
        }

        // 最後の項目への定数ポインタを返します。vector が空の場合は null を返す。
        [[nodiscard]] constexpr const T* end() const
        {
            assert(data_);
            return std::addressof(data_[size_]);
        }

    private:
        constexpr void move(vector& o)
        {
            capacity_ = o.capacity_;
            size_ = o.size_;
            data_ = o.data_;
            o.reset();
        }

        constexpr void reset()
        {
            capacity_ = 0;
            size_ = 0;
            data_ = nullptr;
        }

        constexpr void destruct_range(u64 first, u64 last)
        {
            assert(destruct);
            assert(first <= size_ && last <= size_ && first <= last);
            if (data_)
            {
                for (; first != last; ++first)
                {
                    data_[first].~T();
                }
            }
        }

        constexpr void destroy()
        {
            assert([&] {return capacity_ ? data_ != nullptr : data_ == nullptr; }());
            clear();
            capacity_ = 0;
            if (data_) free(data_);
            data_ = nullptr;
        }

        u64 capacity_{ 0 };
        u64 size_{ 0 };
        T* data_{ nullptr };
    };
}
