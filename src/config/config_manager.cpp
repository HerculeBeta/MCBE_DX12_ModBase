#include "config/config_manager.h"
#include "modules/module_manager.h"
#include "utils/logger.h"
#include <cstdlib>
#include <fstream>

#include <nlohmann/json.hpp> 

ConfigManager& ConfigManager::instance() {
    static ConfigManager manager;
    return manager;
}

std::filesystem::path ConfigManager::get_config_directory() const {
    char* appdata = nullptr;
    size_t len = 0;
    if (_dupenv_s(&appdata, &len, "APPDATA") == 0 && appdata != nullptr) {
        std::filesystem::path path = std::filesystem::path(appdata) / "MCBE_DX12_ModBase" / "configs";
        free(appdata);
        return path;
    }
    return std::filesystem::current_path() / "configs";
}

void ConfigManager::ensure_directory_exists() const {
    std::filesystem::create_directories(get_config_directory());
}

bool ConfigManager::save_config(const std::string& configName) {
    try {
        ensure_directory_exists();
        std::filesystem::path filePath = get_config_directory() / (configName + ".json");

        nlohmann::json root = nlohmann::json::object();
        nlohmann::json modulesJson = nlohmann::json::object();

        for (Module* module : ModuleManager::instance().modules()) {
            if (!module) continue;

            nlohmann::json modData;
            modData["enabled"] = module->enabled();
            modData["keybind"] = module->keybind();
            modData["hold_to_enable"] = module->hold_to_enable();

            nlohmann::json settingsJson = nlohmann::json::object();
            for (const auto& setting : module->settings()) {
                if (setting) {
                    settingsJson[setting->name()] = setting->serialize();
                }
            }
            modData["settings"] = settingsJson;

            modulesJson[module->name()] = modData;
        }

        root["modules"] = modulesJson;

        std::ofstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for saving config: '{}'", configName);
            return false;
        }

        file << root.dump(4);
        LOG_INFO("Config profile '{}' saved successfully.", configName);
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception occurred while saving config '{}': {}", configName, e.what());
        return false;
    }
}

bool ConfigManager::load_config(const std::string& configName) {
    try {
        std::filesystem::path filePath = get_config_directory() / (configName + ".json");
        if (!std::filesystem::exists(filePath)) {
            LOG_WARN("Config profile '{}' does not exist.", configName);
            return false;
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file: '{}'", configName);
            return false;
        }

        nlohmann::json root;
        file >> root;

        if (!root.contains("modules") || !root["modules"].is_object()) {
            LOG_ERROR("Invalid config file structure for profile: '{}'", configName);
            return false;
        }

        const auto& modulesJson = root["modules"];

        for (Module* module : ModuleManager::instance().modules()) {
            if (!module || !modulesJson.contains(module->name())) continue;

            const auto& modData = modulesJson[module->name()];

            if (modData.contains("enabled") && modData["enabled"].is_boolean()) {
                module->set_enabled(modData["enabled"].get<bool>());
            }
            if (modData.contains("keybind") && modData["keybind"].is_number_integer()) {
                module->set_keybind(modData["keybind"].get<int>());
            }
            if (modData.contains("hold_to_enable") && modData["hold_to_enable"].is_boolean()) {
                module->set_hold_to_enable(modData["hold_to_enable"].get<bool>());
            }

            if (modData.contains("settings") && modData["settings"].is_object()) {
                const auto& settingsJson = modData["settings"];
                for (const auto& setting : module->settings()) {
                    if (setting && settingsJson.contains(setting->name())) {
                        setting->deserialize(settingsJson[setting->name()]);
                    }
                }
            }
        }

        LOG_INFO("Config profile '{}' loaded successfully.", configName);
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception occurred while loading config '{}': {}", configName, e.what());
        return false;
    }
}

bool ConfigManager::delete_config(const std::string& configName) {
    try {
        std::filesystem::path filePath = get_config_directory() / (configName + ".json");
        if (std::filesystem::exists(filePath)) {
            bool removed = std::filesystem::remove(filePath);
            if (removed) {
                LOG_INFO("Deleted config profile: '{}'", configName);
            }
            return removed;
        }
        return false;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to delete config profile '{}': {}", configName, e.what());
        return false;
    }
}

std::vector<std::string> ConfigManager::get_available_configs() const {
    std::vector<std::string> configs;
    try {
        ensure_directory_exists();
        for (const auto& entry : std::filesystem::directory_iterator(get_config_directory())) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                configs.push_back(entry.path().stem().string());
            }
        }
    }
    catch (...) {}
    return configs;
}