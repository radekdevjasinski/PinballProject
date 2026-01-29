#pragma once

#include <cmath>
#include <GL/freeglut.h>

#define M_PI 3.14159265358979323846


class Camera {
public:

    float distance = 60.0f; 
    float yaw = 45.0f;     
    float pitch = -60.0f;   

    float centerX = 0.0f;
    float centerY = 10.0f;
    float centerZ = 0.0f;


    float movementSpeed = 0.5f;
    float mouseSensitivity = 0.2f;


    bool isDragging = false;
    int lastMouseX = 0;
    int lastMouseY = 0;

    Camera();

    void setupView(); 
    void updateCenter(float dx, float dy, float dz);

    void processMouseClick(int button, int state, int x, int y);
    void processMouseMove(int x, int y);
    void processKeyboard(unsigned char key);

private:
    void calculatePosition(float& outX, float& outY, float& outZ);
};
