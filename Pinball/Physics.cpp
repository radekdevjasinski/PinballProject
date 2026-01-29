#include "Physics.hpp"
#include "GameObject.hpp"
#include <iostream>

Physics::Physics() {}
Physics::~Physics() {}

void Physics::initialize()
{
    foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);

    PxSceneDesc sceneDesc(physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    PxDefaultCpuDispatcher* dispatcher = PxDefaultCpuDispatcherCreate(1);
    sceneDesc.cpuDispatcher = dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    scene = physics->createScene(sceneDesc);
    scene->setSimulationEventCallback(this);

	cookingParams = new PxCookingParams(physics->getTolerancesScale());

	cookingParams->meshWeldTolerance = 0.001f;
	cookingParams->meshPreprocessParams = PxMeshPreprocessingFlag::eWELD_VERTICES;

    material = physics->createMaterial(0.5f, 0.5f, 0.5f);
}

void Physics::createSceneObjects(GameObject& groundObj, GameObject& ballObj)
{
    PxRigidStatic* planeActor = PxCreatePlane(*physics, PxPlane(0, 1, 0, 0), *material);
    groundObj.actor = planeActor;
    groundObj.position = PxVec3(0, -20, 20);


    // pilka
    ballObj.radius = 0.666f;
    PxVec3 localPos(-8.5f, 7.0f, -21.0f);
    PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));
    PxVec3 worldPos = tableRotation.rotate(localPos);
    ballObj.position = worldPos;
    PxSphereGeometry sphereGeom(ballObj.radius);
    PxTransform startTransform(worldPos, tableRotation);

    PxRigidDynamic* sphereActor = PxCreateDynamic(*physics, startTransform, sphereGeom, *material, 1.0f);
    if (sphereActor)
    {
        //precyzja
        sphereActor->setSolverIterationCounts(12, 4);

        // wypychanie z kolizji
        sphereActor->setMaxDepenetrationVelocity(10.0f);

        // ci¹g³a detekcja
        sphereActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

        // opór powietrza
        sphereActor->setAngularDamping(0.3f);
        sphereActor->setLinearDamping(0.1f);

        sphereActor->setMass(5.0f);
        sphereActor->setName("BALL");

        // contact offset
        PxShape* shapes[1];
        sphereActor->getShapes(shapes, 1);
        shapes[0]->setContactOffset(0.02f);
        shapes[0]->setRestOffset(0.0f);
        scene->addActor(*sphereActor);
        ballObj.actor = sphereActor;
    }
}

void Physics::createTriggers() {
    PxMaterial* triggerMat = physics->createMaterial(0.5f, 0.5f, 0.5f);

    PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));

    // drain
    PxVec3 drainLocalPos(1.0f, 5.0f, -22.0f); 
    PxVec3 drainWorldPos = tableRotation.rotate(drainLocalPos);
    drainTrigger = physics->createRigidStatic(PxTransform(drainWorldPos, tableRotation));
    PxBoxGeometry drainGeom(5.0f, 3.0f, 2.0f); 
    PxShape* drainShape = PxRigidActorExt::createExclusiveShape(*drainTrigger, drainGeom, *triggerMat);
    
    drainShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    drainShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    drainTrigger->setName("DRAIN"); 
    scene->addActor(*drainTrigger);


    // plunger
    
    PxVec3 plungerLocalPos(-8.5f, 7.0f, -21.0f);
    PxVec3 plungerWorldPos = tableRotation.rotate(plungerLocalPos);
    plungerTrigger = physics->createRigidStatic(PxTransform(plungerWorldPos, tableRotation));
    PxBoxGeometry plungerGeom(1.0f, 1.0f, 1.0f);
    PxShape* plungerShape = PxRigidActorExt::createExclusiveShape(*plungerTrigger, plungerGeom, *triggerMat);
    
    plungerShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    plungerShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    plungerTrigger->setName("PLUNGER"); 
    scene->addActor(*plungerTrigger);
}

void Physics::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; i++)
    {
        if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            PxActor* triggerActor = pairs[i].triggerActor;
            PxActor* otherActor = pairs[i].otherActor;

            if (triggerActor == drainTrigger)
            {
                std::cout << "PILKA W DZIURZE! Kolejkowanie resetu..." << std::endl;
                actorToReset = reinterpret_cast<PxRigidActor*>(otherActor);
            }
            else if (triggerActor == plungerTrigger)
            {
                std::cout << "Pilka gotowa do wystrzalu!" << std::endl;
                ballInPlungerActor = otherActor->is<PxRigidDynamic>();
                currentLaunchPower = 0.0f;
            }
        }
    }
}
void Physics::updatePlunger(bool spacePressed, float dt)
{
    if (!ballInPlungerActor) return;

    PxTransform plungerPose = plungerTrigger->getGlobalPose();
    PxTransform ballPose = ballInPlungerActor->getGlobalPose();

    if ((plungerPose.p - ballPose.p).magnitude() > 3.0f) {
        ballInPlungerActor = nullptr;
        currentLaunchPower = 0.0f;
        return;
    }
    // naciaganie
    if (spacePressed)
    {
        currentLaunchPower += CHARGE_SPEED * dt;
        if (currentLaunchPower > MAX_LAUNCH_POWER) currentLaunchPower = MAX_LAUNCH_POWER;

        std::cout << "Naci¹g: " << currentLaunchPower << std::endl; 
        wasSpacePressed = true;
    }
    //strzal
    else if (wasSpacePressed)
    {
        std::cout << "WYSTRZA£ z si³¹: " << currentLaunchPower << std::endl;
        PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));
        PxVec3 launchDir = tableRotation.rotate(PxVec3(0.0f, 0.0f, -1.0f));

        ballInPlungerActor->addForce(launchDir * currentLaunchPower, PxForceMode::eIMPULSE);

        currentLaunchPower = 0.0f;
        wasSpacePressed = false;
    }
}

void Physics::step(float deltaTime)
{
    timer += deltaTime;

    if (timer > 0.1f) timer = 0.1f;

    while (timer >= FIXED_TIMESTEP)
    {
        if (scene)
        {
            scene->simulate(FIXED_TIMESTEP);
            scene->fetchResults(true);

            //reset
            if (actorToReset != nullptr)
            {
                resetBall(actorToReset);
                actorToReset = nullptr;
            }
        }
        timer -= FIXED_TIMESTEP;
    }
    
}

std::vector<PxTransform> Physics::getDynamicTransforms() const
{
    std::vector<PxTransform> transforms;
    if (sphereBody)
    {
        transforms.push_back(sphereBody->getGlobalPose());
    }
    return transforms;
}

PxRigidStatic* Physics::createStaticPinballTable(GameObject& obj) {

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    obj.model->getGeometryData(vertices, indices);

    if (vertices.empty() || indices.empty()) {
        std::cerr << "Brak danych geometrii do utworzenia siatki fizycznej." << std::endl;
        return NULL;
    }

    // tworzenie trójk¹tnej siatki
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = vertices.size() / 3;
    meshDesc.points.stride = sizeof(float) * 3; 
    meshDesc.points.data = vertices.data();    

    meshDesc.triangles.count = indices.size() / 3; 
    meshDesc.triangles.stride = sizeof(uint32_t) * 3;
    meshDesc.triangles.data = indices.data();

    PxTriangleMesh* triMesh = PxCreateTriangleMesh(*cookingParams, meshDesc, physics->getPhysicsInsertionCallback());
    if (!triMesh) {
        std::cerr << "B³¹d gotowania siatki!" << std::endl;
        return NULL;
    }

    // geometria i materia³
    PxTriangleMeshGeometry geometry(triMesh);

    PxMaterial* material = physics->createMaterial(0.9f, 0.9f, 0.9f); 
    PxTransform tablePose(obj.position, obj.rotation);
    PxRigidStatic* staticBody = physics->createRigidStatic(tablePose);
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*staticBody, geometry, *material);

    scene->addActor(*staticBody);
	return staticBody;
}
PxConvexMesh* Physics::createConvexMesh(const std::vector<float>& verts) {
    if (verts.empty()) {
        std::cerr << "Puste wierzcholki dla Convex Mesh!" << std::endl;
        return nullptr;
    }

    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = verts.size() / 3;
    convexDesc.points.stride = sizeof(float) * 3;
    convexDesc.points.data = verts.data();

    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = PxCreateConvexMesh(*cookingParams, convexDesc, physics->getPhysicsInsertionCallback());

    if (!convexMesh) {
        std::cerr << "Blad tworzenia Convex Mesh (PxCreateConvexMesh)!" << std::endl;
    }

    return convexMesh;
}

void Physics::resetBall(PxRigidActor* actor) {
    if (!actor) return;

    PxRigidDynamic* dynamicBall = actor->is<PxRigidDynamic>();

    if (dynamicBall) {
        // reset predkosci
        dynamicBall->setLinearVelocity(PxVec3(0, 0, 0));
        dynamicBall->setAngularVelocity(PxVec3(0, 0, 0));

        // pozycja startowa
        PxVec3 localPos(-8.5f, 7.0f, -21.0f);
        PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));
        PxVec3 worldPos = tableRotation.rotate(localPos);
        dynamicBall->setGlobalPose(PxTransform(worldPos));
        dynamicBall->clearForce();
    }
}

void Physics::createFlippers(GameObject& flipperLObj, GameObject& flipperRObj) {
    std::vector<float> vertsL, vertsR;
    std::vector<uint32_t> indsL, indsR;

    flipperLObj.model->getFlipperLGeometry(vertsL, indsL);
    flipperRObj.model->getFlipperRGeometry(vertsR, indsR);

    const PxVec3 blenderOriginL(0.0f, 0.0f, 0.0f);
    const PxVec3 blenderOriginR(0.0f, 0.0f, 0.0f);

    PxVec3 targetPosL(7.042f, 7.98f, -17.39f);
    PxVec3 targetPosR(-4.149f, 7.98f, -17.39f);

    PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));
    PxVec3 finalPosL = tableRotation.rotate(targetPosL);
    PxVec3 finalPosR = tableRotation.rotate(targetPosR);
    

    for (size_t i = 0; i < vertsL.size(); i += 3) {
        vertsL[i] -= blenderOriginL.x;
        vertsL[i + 1] -= blenderOriginL.y;
        vertsL[i + 2] -= blenderOriginL.z;
    }

    for (size_t i = 0; i < vertsR.size(); i += 3) {
        vertsR[i] -= blenderOriginR.x;
        vertsR[i + 1] -= blenderOriginR.y;
        vertsR[i + 2] -= blenderOriginR.z;
    }

    PxConvexMesh* meshL = createConvexMesh(vertsL);
    PxConvexMesh* meshR = createConvexMesh(vertsR);

    if (!meshL || !meshR) return;

    PxMaterial* flipperMat = physics->createMaterial(0.6f, 0.6f, 0.5f);

	// lewy flipper
    leftFlipperActor = physics->createRigidDynamic(PxTransform(finalPosL, tableRotation));

    leftFlipperActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    PxConvexMeshGeometry geomL(meshL);
    PxShape* shapeL = PxRigidActorExt::createExclusiveShape(*leftFlipperActor, geomL, *flipperMat);
    scene->addActor(*leftFlipperActor);
    flipperLObj.actor = leftFlipperActor;

	// prawy flipper
    rightFlipperActor = physics->createRigidDynamic(PxTransform(finalPosR, tableRotation));

    rightFlipperActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

    PxConvexMeshGeometry geomR(meshR);
    PxShape* shapeR = PxRigidActorExt::createExclusiveShape(*rightFlipperActor, geomR, *flipperMat);
    scene->addActor(*rightFlipperActor);
    flipperRObj.actor = rightFlipperActor;
}
void Physics::updateFlippersInput(bool leftInput, bool rightInput, float dt) {
    if (!leftFlipperActor || !rightFlipperActor) return;
    // lewy flipper
    if (leftInput) {
        // gora
        currentAngleL += FLIPPER_SPEED * dt;
        if (currentAngleL > FLIPPER_MAX_ANGLE) currentAngleL = FLIPPER_MAX_ANGLE;
    }
    else {
        // dol
        currentAngleL -= FLIPPER_SPEED * dt;
        if (currentAngleL < 0.0f) currentAngleL = 0.0f;
    }

	// prawy flipper
    if (rightInput) {
        currentAngleR += FLIPPER_SPEED * dt;
        if (currentAngleR > FLIPPER_MAX_ANGLE) currentAngleR = FLIPPER_MAX_ANGLE;
    }
    else {
        currentAngleR -= FLIPPER_SPEED * dt;
        if (currentAngleR < 0.0f) currentAngleR = 0.0f;
    }

    // fizyka
    PxQuat tableRotation(PxPiDivFour, PxVec3(0, 1, 0));

    PxVec3 basePosL(7.042f, 7.98f, -17.39f);
    PxVec3 basePosR(-4.149f, 7.98f, -17.39f);
    PxVec3 worldPosL = tableRotation.rotate(basePosL);
    PxVec3 worldPosR = tableRotation.rotate(basePosR);

    PxQuat swingL(currentAngleL, PxVec3(0, 1, 0));
    PxQuat swingR(-currentAngleR, PxVec3(0, 1, 0));

    PxQuat finalRotL = tableRotation * swingL;
    PxQuat finalRotR = tableRotation * swingR;

    leftFlipperActor->setKinematicTarget(PxTransform(worldPosL, finalRotL));
    rightFlipperActor->setKinematicTarget(PxTransform(worldPosR, finalRotR));
}

void Physics::cleanup()
{
    if (scene) scene->release();
    if (physics) physics->release();
    if (foundation) foundation->release();
}
