#pragma once

#include <vector>
#include <GL/freeglut.h>
#include "Physics.hpp"
#include "ModelManager.hpp"
#include "Camera.hpp"
#include "GameObject.hpp"

const float RENDER_SPHERE_RADIUS = 1.0f;

class Renderer
{
public:
    Renderer(Physics* engine);

    void run(int argc, char** argv);

    static void displayWrapper();
    static void reshapeWrapper(int w, int h);
    static void keyboardWrapper(unsigned char key, int x, int y);
    static void mouseWrapper(int button, int state, int x, int y);
	static void mouseMoveWrapper(int x, int y);
    static void timerWrapper(int value);

    GameObject pinballTable{ ObjectType::MODEL };
    GameObject ball{ ObjectType::SPHERE };
    GameObject ground{ ObjectType::PLANE };

private:
    Physics* engine;
    Camera camera;

    static Renderer* Instance;

    void setupOpenGL();

    void renderScene();
    void renderGroundPlane();
    void renderDynamicObjects();
    void renderModel();


    void idleUpdate();
	void keyboardInput(unsigned char key, int x, int y);
	void mouseInput(int button, int state, int x, int y);
	void mouseMove(int x, int y);
	void timerUpdate();

    int fps = 60;

	float time = 0.0f;
};