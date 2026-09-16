#pragma once

#include "events/event.h"

class ClientInstance;

struct ClientInstanceUpdateEvent : public Event {
    ClientInstance* clientInstance = nullptr;
    bool isInitFinished = false;

    ClientInstanceUpdateEvent(ClientInstance* instance, bool initFinished)
        : clientInstance(instance), isInitFinished(initFinished) {
    }
};