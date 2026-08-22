#pragma once

#include "utils/math.h"
#include <cstdint>
#include <vector>

#include <minecraft/sdk/components/AABBShapeComponent.h>
#include <minecraft/sdk/components/RenderPositionComponent.h>
#include <minecraft/sdk/components/ActorHeadRotationComponent.h>
#include <minecraft/sdk/components/RuntimeIDComponent.h>
#include "minecraft/sdk/entity/entity_context.h"
#include "minecraft/sdk/components/StateVectorComponent.h"
#include "minecraft/sdk/components/ActorRotationComponent.h"
#include <minecraft/sdk/components/ActorDataFlagComponent.h>
#include <minecraft/sdk/components/ActorGameTypeComponent.h>
#include <minecraft/sdk/components/ActorTypeComponent.h>
#include <minecraft/sdk/components/ActorOwnerComponent.h>
#include <minecraft/sdk/components/ActorUniqueIDComponent.h>

class Actor {
public:
    virtual ~Actor() = default;

    EntityContext* getEntityContext() const {
        if (!this) return nullptr;
        return reinterpret_cast<EntityContext*>(reinterpret_cast<std::uintptr_t>(this) + 0x8);
    }

    template <typename T>
    bool hasComponent() const {
        EntityContext* context = getEntityContext();
        if (!context || !context->mRegistry) return false;
        return context->getRegistry().all_of<T>(context->mEntity);
    }

    template <typename T>
    T* getComponent() const {
        EntityContext* context = getEntityContext();
        if (!context || !context->mRegistry) return nullptr;
        return context->getRegistry().try_get<T>(context->mEntity);
    }

    template <typename T>
    void addComponent() {
        EntityContext* context = getEntityContext();
        if (!context || !context->mRegistry) return;
        context->getRegistry().get_or_emplace<T>(context->mEntity);
    }

    template <typename T>
    void removeComponent() {
        EntityContext* context = getEntityContext();
        if (!context || !context->mRegistry) return;
        context->getRegistry().remove<T>(context->mEntity);
    }

    std::uint64_t getUniqueId() const {
        if (auto* unique = getComponent<ActorUniqueIDComponent>()) {
            return unique->mUniqueID;
        }
        return 0;
    }

    ActorType getActorType() const {
        auto* comp = getComponent<ActorTypeComponent>();
        return comp ? comp->mType : ActorType::Undefined;
    }

    GameType getGameType() const {
        auto* comp = getComponent<ActorGameTypeComponent>();
        return comp ? comp->mGameType : GameType::Undefined;
    }

    void setGameType(GameType gameType) {
        if (auto* comp = getComponent<ActorGameTypeComponent>()) {
            comp->mGameType = gameType;
        }
    }

    bool getFlag(ActorFlags flag) const {
        if (auto* flags = getComponent<ActorDataFlagComponent>()) {
            return flags->getFlag(flag);
        }
        return false;
    }

    void setFlag(ActorFlags flag, bool value) {
        if (auto* flags = getComponent<ActorDataFlagComponent>()) {
            flags->setFlag(flag, value);
        }
    }

    math::Vec3 getVelocity() const {
        auto* stateVector = getComponent<StateVectorComponent>();
        return stateVector ? stateVector->mPosDelta : math::Vec3{ 0.0f, 0.0f, 0.0f };
    }

    void setVelocity(const math::Vec3& velocity) {
        if (auto* stateVector = getComponent<StateVectorComponent>()) {
            stateVector->mPosDelta = velocity;
        }
    }

    math::ViewAngles getRotation() const {
        if (auto* rot = getComponent<ActorRotationComponent>()) {
            return rot->mRotation;
        }
        return { 0.0f, 0.0f };
    }

    void setRotation(const math::ViewAngles& angles) {
        if (auto* rot = getComponent<ActorRotationComponent>()) {
            rot->mRotation = angles;
        }
    }

    float getHeadYaw() const {
        if (auto* headRot = getComponent<ActorHeadRotationComponent>()) {
            return headRot->mHeadYaw.x;
        }
        return getRotation().yaw;
    }

    void setHeadYaw(float headYaw) {
        if (auto* headRot = getComponent<ActorHeadRotationComponent>()) {
            headRot->mHeadYaw.x = headYaw;
        }
    }

    std::uint64_t getRuntimeId() const {
        if (auto* runtime = getComponent<RuntimeIDComponent>()) {
            return runtime->mRuntimeId;
        }
        return 0;
    }

    AABBShapeComponent* getAABB() const {
        return getComponent<AABBShapeComponent>();
    }

    math::Vec3 getPosition() const {
        if (auto* stateVector = getComponent<StateVectorComponent>()) {
            return stateVector->mPos;
        }
        return { 0.0f, 0.0f, 0.0f };
    }

    math::Vec3 getRenderPosition() const {
        if (auto* renderPos = getComponent<RenderPositionComponent>()) {
            return renderPos->mPosition;
        }
        return { 0.0f, 0.0f, 0.0f };
    }

    [[nodiscard]] bool isPlayer() const {
        return getActorType() == ActorType::Player;
    }


    [[nodiscard]] static std::vector<Actor*> get_all(Actor* referenceActor) {
        std::vector<Actor*> actors;
        if (!referenceActor) return actors;

        EntityContext* context = referenceActor->getEntityContext();
        if (!context || !context->mRegistry) return actors;

        auto& registry = context->getRegistry();
        auto view = registry.view<ActorOwnerComponent>();

        for (auto entity : view) {
            auto* owner = registry.try_get<ActorOwnerComponent>(entity);
            if (owner && owner->mActor) {
                actors.push_back(owner->mActor);
            }
        }

        return actors;
    }
};