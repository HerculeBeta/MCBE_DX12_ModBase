#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <atomic>
#include <shared_mutex>
#include <cstring>
#include <imgui.h>

#include <nlohmann/json.hpp> 
#include "ui/widgets/widgets.h"

namespace config {

    class Setting {
    public:
        Setting(std::string name, std::string description, std::function<bool()> visibility = nullptr)
            : m_name(std::move(name)), m_description(std::move(description)), m_visibility(std::move(visibility)) {
        }
        virtual ~Setting() = default;

        const std::string& name() const { return m_name; }
        const std::string& description() const { return m_description; }
        bool is_visible() const { return m_visibility ? m_visibility() : true; }

        virtual void render_ui() = 0;
        virtual nlohmann::json serialize() const = 0;
        virtual void deserialize(const nlohmann::json& j) = 0;

    protected:
        std::string m_name;
        std::string m_description;
        std::function<bool()> m_visibility;
    };

    class BoolSetting : public Setting {
    public:
        BoolSetting(std::string name, std::string description, bool defaultValue, std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)), m_value(defaultValue) {
        }

        bool value() const { return m_value.load(std::memory_order_acquire); }
        void set_value(bool val) { m_value.store(val, std::memory_order_release); }
        operator bool() const { return value(); }

        void render_ui() override {
            bool val = m_value.load(std::memory_order_relaxed);
            if (ui::ToggleSwitch(m_name.c_str(), &val)) {
                m_value.store(val, std::memory_order_release);
            }
            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override { return value(); }
        void deserialize(const nlohmann::json& j) override {
            if (j.is_boolean()) set_value(j.get<bool>());
        }

    private:
        std::atomic<bool> m_value;
    };

    class FloatSetting : public Setting {
    public:
        FloatSetting(std::string name, std::string description, float defaultValue, float min, float max, const char* format = "%.2f", std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)),
            m_value(defaultValue), m_min(min), m_max(max), m_format(format) {
        }

        float value() const { return m_value.load(std::memory_order_acquire); }
        void set_value(float val) { m_value.store(std::clamp(val, m_min, m_max), std::memory_order_release); }
        operator float() const { return value(); }

        void render_ui() override {
            float val = m_value.load(std::memory_order_relaxed);
            if (ui::SliderFloat(m_name.c_str(), &val, m_min, m_max, m_format)) {
                set_value(val);
            }
            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override { return value(); }
        void deserialize(const nlohmann::json& j) override {
            if (j.is_number()) set_value(j.get<float>());
        }

    private:
        std::atomic<float> m_value;
        float m_min;
        float m_max;
        const char* m_format;
    };

    class IntSetting : public Setting {
    public:
        IntSetting(std::string name, std::string description, int defaultValue, int min, int max, std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)),
            m_value(defaultValue), m_min(min), m_max(max) {
        }

        int value() const { return m_value.load(std::memory_order_acquire); }
        void set_value(int val) { m_value.store(std::clamp(val, m_min, m_max), std::memory_order_release); }
        operator int() const { return value(); }

        void render_ui() override {
            int val = m_value.load(std::memory_order_relaxed);
            if (ui::SliderInt(m_name.c_str(), &val, m_min, m_max)) {
                set_value(val);
            }
            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override { return value(); }
        void deserialize(const nlohmann::json& j) override {
            if (j.is_number_integer()) set_value(j.get<int>());
        }

    private:
        std::atomic<int> m_value;
        int m_min;
        int m_max;
    };

    class EnumSetting : public Setting {
    public:
        EnumSetting(std::string name, std::string description, int defaultValue, std::vector<std::string> options, std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)),
            m_value(defaultValue), m_options(std::move(options)) {
        }

        int value() const { return m_value.load(std::memory_order_acquire); }
        void set_value(int val) {
            if (val >= 0 && val < static_cast<int>(m_options.size())) {
                m_value.store(val, std::memory_order_release);
            }
        }
        operator int() const { return value(); }

        std::string current_option() const {
            int val = value();
            if (val >= 0 && val < static_cast<int>(m_options.size())) {
                return m_options[val];
            }
            return "";
        }

        void render_ui() override {
            int val = m_value.load(std::memory_order_relaxed);
            if (ui::Combo(m_name.c_str(), &val, m_options)) {
                set_value(val);
            }
            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override { return value(); }
        void deserialize(const nlohmann::json& j) override {
            if (j.is_number_integer()) {
                set_value(j.get<int>());
            }
        }

    private:
        std::atomic<int> m_value;
        std::vector<std::string> m_options;
    };

    class ColorSetting : public Setting {
    public:
        ColorSetting(std::string name, std::string description, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f, std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)), m_color{ r, g, b, a } {
        }

        void get_color(float outColor[4]) const {
            std::shared_lock lock(m_mutex);
            std::memcpy(outColor, m_color, sizeof(float) * 4);
        }

        void set_color(float r, float g, float b, float a) {
            std::unique_lock lock(m_mutex);
            m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = a;
        }

        ImVec4 as_imvec4() const {
            std::shared_lock lock(m_mutex);
            return ImVec4(m_color[0], m_color[1], m_color[2], m_color[3]);
        }

        void render_ui() override {
            float tempColor[4];
            {
                std::shared_lock lock(m_mutex);
                std::memcpy(tempColor, m_color, sizeof(tempColor));
            }

            if (ui::ColorPicker(m_name.c_str(), tempColor)) {
                std::unique_lock lock(m_mutex);
                std::memcpy(m_color, tempColor, sizeof(m_color));
            }

            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override {
            std::shared_lock lock(m_mutex);
            return nlohmann::json::array({ m_color[0], m_color[1], m_color[2], m_color[3] });
        }

        void deserialize(const nlohmann::json& j) override {
            if (j.is_array() && j.size() == 4) {
                std::unique_lock lock(m_mutex);
                m_color[0] = j[0].get<float>();
                m_color[1] = j[1].get<float>();
                m_color[2] = j[2].get<float>();
                m_color[3] = j[3].get<float>();
            }
        }

    private:
        mutable std::shared_mutex m_mutex;
        float m_color[4];
    };

    class StringSetting : public Setting {
    public:
        StringSetting(std::string name, std::string description, std::string defaultValue, std::function<bool()> visibility = nullptr)
            : Setting(std::move(name), std::move(description), std::move(visibility)), m_value(std::move(defaultValue)) {
        }

        std::string value() const {
            std::shared_lock lock(m_mutex);
            return m_value;
        }

        void set_value(std::string val) {
            std::unique_lock lock(m_mutex);
            m_value = std::move(val);
        }

        void render_ui() override {
            char buffer[256] = {};
            {
                std::shared_lock lock(m_mutex);
                strncpy_s(buffer, m_value.c_str(), sizeof(buffer) - 1);
            }

            if (ui::InputText(m_name.c_str(), buffer, sizeof(buffer))) {
                std::unique_lock lock(m_mutex);
                m_value = buffer;
            }

            if (!m_description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", m_description.c_str());
            }
        }

        nlohmann::json serialize() const override {
            std::shared_lock lock(m_mutex);
            return m_value;
        }

        void deserialize(const nlohmann::json& j) override {
            if (j.is_string()) {
                set_value(j.get<std::string>());
            }
        }

    private:
        mutable std::shared_mutex m_mutex;
        std::string m_value;
    };

}