#pragma once

#include "events/event.h"

struct ClientInstanceUpdateEvent : public Event {
    void* clientInstance = nullptr;
    bool isInitFinished = false;

    ClientInstanceUpdateEvent(void* instance, bool initFinished)
        : clientInstance(instance), isInitFinished(initFinished) {
    }
};