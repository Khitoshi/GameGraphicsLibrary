#pragma once

#include <vector>
#include <deque>
#include <algorithm>

#define USE_STL_VECTOR 1

namespace Container {

#if USE_STL_VECTOR
    template<typename T>
    using vector = std::vector<T>;

    template<typename T>
    void EraseUnordered(std::vector<T>& v, size_t index)
    {
        if (v.size() > 1) {
            std::iter_swap(v.begin() + index, v.end() - 1);
            v.pop_back();
        }
        else {
            v.clear();
        }
    }
#endif
    template<typename T>
    using deque = std::deque<T>;
}