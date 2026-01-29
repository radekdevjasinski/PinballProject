#include "Renderer.hpp"
#include "Physics.hpp"
#include <iostream>
#include <GL/freeglut.h>
#include <PxPhysicsAPI.h> 
using namespace physx;

Renderer* Renderer::Instance = nullptr;

Renderer::Renderer(Physics* engine) : engine(engine)
{
    Instance = this;
	time = glutGet(GLUT_ELAPSED_TIME);
}

void Renderer::run(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 900);
    glutCreateWindow("Pinball");

    pinballTable.model = new ModelManager();

    std::string modelPath = "../../../Models/pinballTable2.obj";

    if (!pinballTable.model->loadModel(modelPath)) {
        std::cerr << "Nie udalo sie zaladowac modelu." << std::endl;
    }
    flipperL.model = pinballTable.model;
    flipperR.model = pinballTable.model;
    pinballTable.rotation = PxQuat(PxPiDivFour, PxVec3(0, 1, 0));
    pinballTable.actor = engine->createStaticPinballTable(pinballTable);
	engine->createFlippers(flipperL, flipperR);
    engine->createTriggers();

    glutDisplayFunc(displayWrapper);
    glutReshapeFunc(reshapeWrapper);
    glutKeyboardFunc(keyboardWrapper);
    glutMouseFunc(mouseWrapper);
	glutMotionFunc(mouseMoveWrapper);
    glutTimerFunc(16, timerWrapper, 0);
    glutKeyboardUpFunc(keyboardUpWrapper);
    
    engine->createSceneObjects(ground, ball);


    setupOpenGL();

    glutMainLoop();
}

void Renderer::setupOpenGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat light_pos[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    GLfloat ambient_light[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
}


void Renderer::renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.setupView();

    ball.update();
    ball.draw();

    // ground.draw();

    // lewy flipper
    if (flipperL.actor) {
        glPushMatrix();
        PxTransform t = flipperL.actor->getGlobalPose();
        PxMat44 m(t);
        float glMat[16];
        for (int i = 0; i < 16; i++) glMat[i] = m.front()[i];
        glMultMatrixf(glMat);

        flipperL.model->drawFlipperL();
        glPopMatrix();
    }

    // prawy flipper
    if (flipperR.actor) {
        glPushMatrix();
        PxTransform t = flipperR.actor->getGlobalPose();
        PxMat44 m(t);
        float glMat[16];
        for (int i = 0; i < 16; i++) glMat[i] = m.front()[i];
        glMultMatrixf(glMat);

        flipperR.model->drawFlipperR();
        glPopMatrix();
    }

    // stol
    if (pinballTable.actor) {
        glPushMatrix();
        PxTransform t = pinballTable.actor->getGlobalPose();
        PxMat44 m(t);
        float glMat[16];
        for (int i = 0; i < 16; i++) glMat[i] = m.front()[i];
        glMultMatrixf(glMat);

        pinballTable.model->drawTable();
        glPopMatrix();
    }

    // debug fizyki
    /*
    
    if (flipperL.actor) drawPhysXActor(flipperL.actor);
    if (flipperR.actor) drawPhysXActor(flipperR.actor);
    if (ball.actor) drawPhysXActor(ball.actor);
    if (pinballTable.actor) drawPhysXActor(pinballTable.actor);
    if (engine->drainTrigger) drawPhysXActor(engine->drainTrigger);
    if (engine->plungerTrigger) drawPhysXActor(engine->plungerTrigger);

    */
    

    glutSwapBuffers();
}
void Renderer::drawPhysXActor(PxRigidActor* actor) {
    if (!actor) return;

    PxU32 nbShapes = actor->getNbShapes();
    std::vector<PxShape*> shapes(nbShapes);
    actor->getShapes(shapes.data(), nbShapes);

    for (PxU32 i = 0; i < nbShapes; i++) {
        PxShape* shape = shapes[i];
        PxGeometryType::Enum type = shape->getGeometry().getType();
        const PxGeometry& geom = shape->getGeometry();

        PxTransform globalPose = actor->getGlobalPose();
        PxTransform localPose = shape->getLocalPose();
        PxTransform finalPose = globalPose * localPose;

        PxMat44 m(finalPose);
        float glMat[16];
        for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) glMat[c * 4 + r] = m[c][r];

        glPushMatrix();
        glMultMatrixf(glMat);

        // flippery
        if (type == PxGeometryType::eCONVEXMESH) {
            const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);
            PxConvexMesh* mesh = convexGeom.convexMesh;

            glDisable(GL_LIGHTING);
            glLineWidth(2.0f);
            glColor3f(0.0f, 1.0f, 0.0f); // Zielony

            if (mesh) {
                PxU32 nbPolys = mesh->getNbPolygons();
                const PxVec3* verts = mesh->getVertices();
                const PxU8* indices = mesh->getIndexBuffer();

                for (PxU32 j = 0; j < nbPolys; j++) {
                    PxHullPolygon data;
                    mesh->getPolygonData(j, data);

                    glBegin(GL_LINE_LOOP);
                    for (PxU32 k = 0; k < data.mNbVerts; k++) {
                        PxU32 idx = indices[data.mIndexBase + k];
                        glVertex3f(verts[idx].x, verts[idx].y, verts[idx].z);
                    }
                    glEnd();
                }
            }
        }
        // triggery
        else if (type == PxGeometryType::eBOX) {
            const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);

            glDisable(GL_LIGHTING);
            glLineWidth(2.0f);
            glColor3f(1.0f, 0.0f, 0.0f); // Czerwony

            glPushMatrix();
            glScalef(boxGeom.halfExtents.x, boxGeom.halfExtents.y, boxGeom.halfExtents.z);
            glutWireCube(2.0f);
            glPopMatrix();
        }

        // przywracanie stanu OpenGL
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
}
void Renderer::keyboardInput(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') leftFlipperPressed = true;

    if (key == 'e' || key == 'E') rightFlipperPressed = true;

    if (key == 'r' || key == 'R') engine->resetBall(ball.actor);

    if (key == 32) { // spacja
        spacePressed = true;
    }

    camera.processKeyboard(key);
    glutPostRedisplay();

}
void Renderer::keyboardUpInput(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') leftFlipperPressed = false;
    if (key == 'e' || key == 'E') rightFlipperPressed = false;

    if (key == 32) { // spacja
        spacePressed = false;
    }
}

void Renderer::mouseInput(int button, int state, int x, int y) {
    camera.processMouseClick(button, state, x, y);
    glutPostRedisplay();
}

void Renderer::mouseMove(int x, int y)
{
    camera.processMouseMove(x, y);
    glutPostRedisplay(); 
}

void Renderer::timerUpdate()
{

	float dt = (glutGet(GLUT_ELAPSED_TIME) - time) / 1000.0f;

	time = glutGet(GLUT_ELAPSED_TIME);

	if (dt <= 0.0f) dt = 0.001f;
	if (dt > 0.1f) dt = 0.1f;
    
    engine->updateFlippersInput(leftFlipperPressed, rightFlipperPressed, dt);
    engine->updatePlunger(spacePressed, dt);
    engine->step(dt);

    if (ball.actor) {
        PxTransform ballPose = ball.actor->getGlobalPose();


        if (ballPose.p.y < -10.0f) {
            std::cout << "Game Over! Resetuje pilke..." << std::endl;
            engine->resetBall(ball.actor);
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timerWrapper, 0);
}


void Renderer::displayWrapper()
{
    if (Instance) Instance->renderScene();
}

void Renderer::reshapeWrapper(int w, int h)
{
    if (Instance)
    {
        glViewport(0, 0, w, h);
		Instance->camera.setupView();
    }
}
void Renderer::keyboardWrapper(unsigned char key, int x, int y)
{
    if (Instance) Instance->keyboardInput(key, x, y);
}
void Renderer::mouseWrapper(int button, int state, int x, int y)
{
    if (Instance) Instance->mouseInput(button, state, x, y);
}
void Renderer::timerWrapper(int value)
{
    if (Instance) Instance->timerUpdate();
}
void Renderer::mouseMoveWrapper(int x, int y)
{
    if (Instance) Instance->mouseMove(x, y);
}
void Renderer::keyboardUpWrapper(unsigned char key, int x, int y) {
    if (Instance) Instance->keyboardUpInput(key, x, y);
}

