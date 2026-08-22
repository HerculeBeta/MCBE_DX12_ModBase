#pragma once

#include <Windows.h>

class Client {
public:
    static Client& instance();

    void initialize(HMODULE module);
    void shutdown();

    HMODULE module_handle() const { return m_module; }

private:
    Client() = default;

    HMODULE m_module = nullptr;
    bool m_configLoaded = false;
};