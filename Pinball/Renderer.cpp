#include "Renderer.hpp"
#include "Physics.hpp"
#include <iostream>

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
    glutInitWindowSize(800, 600);
    glutCreateWindow("Pinball");

    pinballTable.model = new ModelManager();

    std::string modelPath = "../../../Models/pinballTable.obj";

    if (!pinballTable.model->loadModel(modelPath)) {
        std::cerr << "Nie udalo sie zaladowac modelu." << std::endl;
    }
    pinballTable.rotation = PxQuat(PxPiDivFour, PxVec3(0, 1, 0));
    pinballTable.actor = engine->createStaticPinballTable(pinballTable);

    glutDisplayFunc(displayWrapper);
    glutReshapeFunc(reshapeWrapper);
    glutKeyboardFunc(keyboardWrapper);
    glutMouseFunc(mouseWrapper);
	glutMotionFunc(mouseMoveWrapper);
    glutTimerFunc(16, timerWrapper, 0);
    
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

	ground.draw();
	pinballTable.draw();


    glutSwapBuffers();
}


void Renderer::keyboardInput(unsigned char key, int x, int y) {
    camera.processKeyboard(key);
    glutPostRedisplay();

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

    engine->step(dt);

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

