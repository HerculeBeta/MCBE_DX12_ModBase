#pragma once

#include <Windows.h>
#include <Psapi.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace minecraft::hook_memory {
inline std::vector<int> pattern_to_bytes(const char* pattern) {
    std::vector<int> bytes;

    for (const char* current = pattern; *current != '\0';) {
        if (*current == ' ') {
            ++current;
            continue;
        }

        if (*current == '?') {
            bytes.push_back(-1);
            ++current;
            if (*current == '?') {
                ++current;
            }
            continue;
        }

        char* next = nullptr;
        bytes.push_back(static_cast<int>(std::strtoul(current, &next, 16)));
        current = next;
    }

    return bytes;
}

inline bool matches_signature(std::uintptr_t address, const std::vector<int>& bytes) {
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (bytes[i] != -1 && *reinterpret_cast<std::uint8_t*>(address + i) != bytes[i]) {
            return false;
        }
    }

    return true;
}

inline std::uintptr_t find_signature(const char* signature) {
    HMODULE module = GetModuleHandleA("Minecraft.Windows.exe");
    if (!module) {
        module = GetModuleHandleA(nullptr);
    }

    MODULEINFO info{};
    if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info))) {
        return 0;
    }

    const std::vector<int> bytes = pattern_to_bytes(signature);
    if (bytes.empty() || info.SizeOfImage < bytes.size()) {
        return 0;
    }

    const auto start = reinterpret_cast<std::uintptr_t>(info.lpBaseOfDll);
    const std::uintptr_t end = start + info.SizeOfImage - bytes.size();

    for (std::uintptr_t address = start; address <= end; ++address) {
        if (matches_signature(address, bytes)) {
            return address;
        }
    }

    return 0;
}

inline std::uintptr_t vtable_entry(void* instance, int index) {
    if (!instance) {
        return 0;
    }

    __try {
        const auto table = *reinterpret_cast<std::uintptr_t**>(instance);
        return table ? table[index] : 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

template <typename Return, typename... Args>
Return call_vtable(void* instance, int index, Args... args) {
    using Fn = Return(__fastcall*)(void*, Args...);
    const std::uintptr_t target = vtable_entry(instance, index);
    if (!target) {
        return Return{};
    }

    return reinterpret_cast<Fn>(target)(instance, args...);
}
}
