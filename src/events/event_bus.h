#pragma once

#include "events/event.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cstdint>
#include <mutex>
#include <atomic>

class EventBus {
public:
    using ListenerId = std::uint64_t;

    template <typename T>
    using EventCallback = std::function<void(T&)>;

    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    template <typename T>
    ListenerId subscribe(EventCallback<T> callback) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");

        const std::type_index typeIndex = typeid(T);
        const ListenerId id = m_nextId.fetch_add(1, std::memory_order_relaxed);

        auto wrapper = [callback](Event& event) {
            callback(static_cast<T&>(event));
            };

        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners[typeIndex].push_back({ id, wrapper });
        return id;
    }

    void unsubscribe(ListenerId id) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [type, list] : m_listeners) {
            std::erase_if(list, [id](const ListenerEntry& entry) {
                return entry.id == id;
                });
        }
    }

    template <typename T>
    void publish(T& event) {
        static_assert(std::is_base_of_v<Event, T>, "T must derive from Event");

        const std::type_index typeIndex = typeid(T);
        std::vector<ListenerEntry> listenersCopy;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_listeners.find(typeIndex);
            if (it == m_listeners.end()) {
                return;
            }
            listenersCopy = it->second;
        }

        for (const auto& entry : listenersCopy) {
            entry.callback(event);
            if (event.is_cancelled()) {
                break;
            }
        }
    }

private:
    EventBus() = default;

    struct ListenerEntry {
        ListenerId id;
        std::function<void(Event&)> callback;
    };

    std::mutex m_mutex;
    std::atomic<ListenerId> m_nextId{ 1 };
    std::unordered_map<std::type_index, std::vector<ListenerEntry>> m_listeners;
};