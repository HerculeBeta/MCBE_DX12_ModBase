#pragma once
#include <cstdint>

namespace utils::memory {
    inline bool is_valid_ptr(const void* ptr) {
        if (!ptr) return false;
        const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
        return (addr >= 0x10000 && addr < 0x7FFFFFFFFFFF);
    }
}
