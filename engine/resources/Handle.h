#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

template<typename T>
struct Handle
{
    std::size_t index = SIZE_MAX;
    bool valid() const { return index != SIZE_MAX; }
};