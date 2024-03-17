#pragma once

#include <vector>
#include <deque>
#include <algorithm>

namespace Container {
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

    template<typename T>
    using deque = std::deque<T>;
}