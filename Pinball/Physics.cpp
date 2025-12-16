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

	cookingParams = new PxCookingParams(physics->getTolerancesScale());

	cookingParams->meshWeldTolerance = 0.001f;
	cookingParams->meshPreprocessParams = PxMeshPreprocessingFlag::eWELD_VERTICES;

    material = physics->createMaterial(0.5f, 0.5f, 0.5f);
}

void Physics::createSceneObjects(GameObject& groundObj, GameObject& ballObj)
{
    PxRigidStatic* planeActor = PxCreatePlane(*physics, PxPlane(0, 1, 0, 0), *material);
    //scene->addActor(*planeActor);

    groundObj.actor = planeActor;
    groundObj.position = PxVec3(0, -20, 20);

	
    
    ballObj.radius = 0.5f;
	ballObj.position = PxVec3(5.0f, 20.0f, 10.0f);

    PxSphereGeometry sphereGeom(ballObj.radius);
    PxTransform startTransform(ballObj.position);

    PxRigidDynamic* sphereActor = PxCreateDynamic(*physics, startTransform, sphereGeom, *material, 1.0f);
    if (sphereActor)
    {
        sphereActor->setAngularDamping(0.5f);
		sphereActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
        scene->addActor(*sphereActor);
		ballObj.actor = sphereActor;
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

    // ekstrakcja danych
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

    // a. Geometria i materia³
    PxTriangleMeshGeometry geometry(triMesh);

    PxMaterial* material = physics->createMaterial(0.9f, 0.9f, 0.9f); 
    PxTransform tablePose(obj.position, obj.rotation);
    PxRigidStatic* staticBody = physics->createRigidStatic(tablePose);
    PxShape* shape = PxRigidActorExt::createExclusiveShape(*staticBody, geometry, *material);

    scene->addActor(*staticBody);
	return staticBody;
}
void Physics::cleanup()
{
    if (scene) scene->release();
    if (physics) physics->release();
    if (foundation) foundation->release();
}
