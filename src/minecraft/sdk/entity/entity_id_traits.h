#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <entt/entt.hpp>
#include "i_entity_component.h"

class EntityId;

struct EntityIdTraits {
    using value_type = EntityId;
    using entity_type = uint32_t;
    using version_type = uint16_t;

    static constexpr entity_type entity_mask = 0x3FFFF;
    static constexpr entity_type version_mask = 0x3FFF;
};

template <typename Type>
struct entt::storage_type<Type, EntityId> {
    using type = basic_storage<Type, EntityId>;
};

template <typename Type>
struct entt::component_traits<Type, std::enable_if_t<std::is_base_of_v<IEntityComponent, Type>>> {
    using type = Type;
    static constexpr bool in_place_delete = true;
    static constexpr std::size_t page_size = 128;
};