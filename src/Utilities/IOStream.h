#pragma once
#include "CommonHeaders.h"

namespace Utility {

    // 注意：（重要）このユーティリティ・クラスは、ローカルでの使用（つまり1つの関数内での使用）のみを目的としている
    // インスタンスをメンバ変数として保持しないでください
    class BlobStreamReader
    {
    public:
        DISABLE_COPY_AND_MOVE(BlobStreamReader);
        explicit BlobStreamReader(const u8* buffer) :
            buffer_{ buffer },
            position_{ buffer }
        {
            assert(buffer);
        }

        // このテンプレート関数は、プリミティブ型（int、float、boolなど）を読み込むためのもの
        template<typename T>
        [[nodiscard]] T Read()
        {
            static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitve type.");
            T value{ *((T*)position_) };
            position_ += sizeof(T);
            return value;
        }

        // lengthバイトをbufferに読み込む。
        // 呼び出し側は、buffer に十分なメモリを確保する必要がある
        void Read(u8* buffer, size_t length)
        {
            memcpy(buffer, position_, length);
            position_ += length;
        }

        void Skip(size_t offset)
        {
            position_ += offset;
        }

        [[nodiscard]] constexpr const u8* const BufferStart() const { return buffer_; }
        [[nodiscard]] constexpr const u8* const Position() const { return position_; }
        [[nodiscard]] constexpr size_t Offset() const { return position_ - buffer_; }
    private:
        const u8* const buffer_;
        const u8* position_;
    };


    // 注意：（重要）このユーティリティ・クラスは、ローカルでの使用（つまり1つの関数内での使用）のみを目的としている
    // インスタンスをメンバ変数として保持しないでください
    class BlobStreamWriter
    {
    public:
        DISABLE_COPY_AND_MOVE(BlobStreamWriter);
        explicit BlobStreamWriter(u8* buffer, size_t buffer_size) :
            buffer_{ buffer },
            position_{ buffer },
            buffer_size_{ buffer_size }
        {
            assert(buffer && buffer_size);
        }

        // このテンプレート関数は、プリミティブ型（int、float、boolなど）を記述するためのもの
        template<typename T>
        void Write(T value)
        {
            static_assert(std::is_arithmetic_v<T>, "Template argument should be a primitve type.");
            assert(&position_[sizeof(T)] <= &buffer_[buffer_size_]);
            *((T*)position_) = value;
            position_ += sizeof(T);
        }

        // length 文字を buffer に書き込む。
        void Write(const char* buffer, size_t length)
        {
            assert(&position_[length] <= &buffer_[buffer_size_]);
            memcpy(position_, buffer, length);
            position_ += length;
        }

        // length バイトを buffer に書き込む。
        void Write(const u8* buffer, size_t length)
        {
            assert(&position_[length] <= &buffer_[buffer_size_]);
            memcpy(position_, buffer, length);
            position_ += length;
        }

        void Skip(size_t offset)
        {
            assert(&position_[offset] <= &buffer_[buffer_size_]);
            position_ += offset;
        }

        [[nodiscard]] constexpr const u8* const BufferStart() const { return buffer_; }
        [[nodiscard]] constexpr const u8* const BufferEnd() const { return &buffer_[buffer_size_]; }
        [[nodiscard]] constexpr const u8* const Position() const { return position_; }
        [[nodiscard]] constexpr size_t Offset() const { return position_ - buffer_; }
    private:
        u8* const   buffer_;
        u8* position_;
        size_t      buffer_size_;
    };
}
