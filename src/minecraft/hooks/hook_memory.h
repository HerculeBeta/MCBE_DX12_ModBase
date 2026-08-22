#pragma once

#include <Windows.h>
#include <cstdint>
#include <string_view>

#include <libhat/scanner.hpp>
#include "utils/logger.h"

namespace minecraft::hook_memory {

    inline std::uintptr_t find_signature(std::string_view signatureStr) {
        const auto parsed = ::hat::parse_signature(signatureStr);
        if (!parsed.has_value()) {
            return 0;
        }

        const auto result = ::hat::find_pattern(parsed.value(), ".text");
        if (!result.has_result() || !result.get()) {
            return 0;
        }

        return reinterpret_cast<std::uintptr_t>(result.get());
    }

    inline std::uintptr_t resolve_signature(const char* name, std::string_view signatureStr) {
        const std::uintptr_t addr = find_signature(signatureStr);
        if (addr) {
            LOG_INFO("[Signature] {:<32} -> OK ({:#x})", name, addr);
        }
        else {
            LOG_ERROR("[Signature] {:<32} -> FAILED!", name);
        }
        return addr;
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

    template <typename Return, typename Instance, typename... Args>
    Return call_function(std::uintptr_t funcAddress, Instance* instance, Args... args) {
        using Fn = Return(__fastcall*)(Instance*, Args...);
        if (!funcAddress || !instance) {
            return Return{};
        }
        return reinterpret_cast<Fn>(funcAddress)(instance, args...);
    }

}