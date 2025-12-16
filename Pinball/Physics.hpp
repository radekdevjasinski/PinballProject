#pragma once

#include "PxPhysicsAPI.h"
#include "ModelManager.hpp"
#include "GameObject.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

using namespace physx;

class Physics
{
public:
    Physics();
    ~Physics();

    void initialize();
    void cleanup();

	void step(float deltaTime);

    std::vector<PxTransform> getDynamicTransforms() const;
    void createSceneObjects(GameObject& groundObj, GameObject& ballObj);
    PxRigidStatic* createStaticPinballTable(GameObject& obj);

private:
    PxDefaultAllocator allocator;
    PxDefaultErrorCallback errorCallback;
    PxFoundation* foundation = nullptr;
    PxPhysics* physics = nullptr;
    PxScene* scene = nullptr;
    PxMaterial* material = nullptr;
	PxCookingParams* cookingParams = nullptr;

    PxRigidStatic* groundPlane = nullptr;
    PxRigidDynamic* sphereBody = nullptr;

    const float FIXED_TIMESTEP = 1.0f / 60.0f;
	float timer = 0.0f;

};