#pragma once

#include "PxPhysicsAPI.h"
#include "ModelManager.hpp"
#include "GameObject.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

using namespace physx;

class Physics : public PxSimulationEventCallback
{
public:
    Physics();
    ~Physics();

    void initialize();
    void cleanup();
	void step(float deltaTime);

    // obiekty
    void createSceneObjects(GameObject& groundObj, GameObject& ballObj);
    PxRigidStatic* createStaticPinballTable(GameObject& obj);
    PxConvexMesh* createConvexMesh(const std::vector<float>& verts);
    std::vector<PxTransform> getDynamicTransforms() const;
    void resetBall(PxRigidActor* ballActor);
    
    
    // flippery
    void createFlippers(GameObject& flipperL, GameObject& flipperR);
    void updateFlippersInput(bool leftInput, bool rightInput, float dt);
    PxRigidDynamic* rightFlipperActor = nullptr;
    PxRigidDynamic* leftFlipperActor = nullptr;
    
    
    // triggery
    void createTriggers();
    void updatePlunger(bool spacePressed, float dt);
    void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
    PxRigidStatic* drainTrigger = nullptr;
    PxRigidStatic* plungerTrigger = nullptr;


    // puste inferejsowe
    void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {}
    void onWake(PxActor** actors, PxU32 count) override {}
    void onSleep(PxActor** actors, PxU32 count) override {}
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {}
    void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}

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

    // flippery
    float currentAngleL = 0.0f;
    float currentAngleR = 0.0f;
    const float FLIPPER_MAX_ANGLE = 50.0f * (3.14159f / 180.0f);
    const float FLIPPER_SPEED = 10.0f; 

    // triggery
    PxRigidActor* actorToReset = nullptr;
    PxRigidDynamic* ballInPlungerActor = nullptr; 
    
    // plunger
    float currentLaunchPower = 0.0f;
    bool wasSpacePressed = false;  
    const float MAX_LAUNCH_POWER = 200.0f;
    const float CHARGE_SPEED = 100.0f;
    


};