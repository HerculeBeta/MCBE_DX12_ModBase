#pragma once

#include "entity_id.h"
#include <entt/entt.hpp>

class EntityRegistry;

class EntityContext {
public:
    EntityRegistry* mRegistry = nullptr;
    entt::basic_registry<EntityId>& mEnttRegistry;
    EntityId mEntity;

    [[nodiscard]] inline entt::basic_registry<EntityId>& getRegistry() {
        return mEnttRegistry;
    }

    [[nodiscard]] inline const entt::basic_registry<EntityId>& getRegistry() const {
        return mEnttRegistry;
    }
};