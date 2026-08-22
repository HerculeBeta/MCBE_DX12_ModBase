#pragma once

#include <string>
#include <vector>
#include <filesystem>

class ConfigManager {
public:
    static ConfigManager& instance();

    std::filesystem::path get_config_directory() const;

    bool save_config(const std::string& configName = "default");
    bool load_config(const std::string& configName = "default");
    bool delete_config(const std::string& configName);

    std::vector<std::string> get_available_configs() const;

private:
    ConfigManager() = default;
    void ensure_directory_exists() const;
};