#include "Camera.hpp"
#include <iostream>

Camera::Camera() {
}

void Camera::calculatePosition(float& outX, float& outY, float& outZ) {
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    float yawRad = yaw * (M_PI / 180.0f);
    float pitchRad = pitch * (M_PI / 180.0f);

    float dirX = cos(yawRad) * cos(pitchRad);
    float dirY = sin(pitchRad);
    float dirZ = sin(yawRad) * cos(pitchRad);

    outX = centerX - dirX * distance;
    outY = centerY - dirY * distance;
    outZ = centerZ - dirZ * distance;
}

void Camera::setupView() {
    float camX, camY, camZ;
    calculatePosition(camX, camY, camZ);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, glutGet(GLUT_WINDOW_WIDTH) / (double)glutGet(GLUT_WINDOW_HEIGHT), 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camX, camY, camZ,      
        centerX, centerY, centerZ, 
        0.0, 1.0, 0.0);      


}

void Camera::updateCenter(float dx, float dy, float dz) {
    centerX += dx;
    centerY += dy;
    centerZ += dz;
}

void Camera::processMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else {
            isDragging = false;
        }
    }

    if (button == 3) { // Scroll up
        distance -= movementSpeed * 5.0f;
    }
    else if (button == 4) { // Scroll down
        distance += movementSpeed * 5.0f;
    }

    if (distance < 2.0f) distance = 2.0f;
    if (distance > 100.0f) distance = 100.0f;
}

void Camera::processMouseMove(int x, int y) {
    if (isDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        yaw += dx * mouseSensitivity;

        pitch -= dy * mouseSensitivity;

        lastMouseX = x;
        lastMouseY = y;
    }
}

void Camera::processKeyboard(unsigned char key) {

    float yawRad = yaw * (M_PI / 180.0f);

    float forwardX = cos(yawRad);
    float forwardZ = sin(yawRad);
    float rightX = cos(yawRad + M_PI / 2.0f);
    float rightZ = sin(yawRad + M_PI / 2.0f);

    switch (key) {
    case 'w': case 'W': // W PRZÓD
        updateCenter(forwardX * movementSpeed, 0.0f, forwardZ * movementSpeed);
        break;
    case 's': case 'S': // W TY£
        updateCenter(-forwardX * movementSpeed, 0.0f, -forwardZ * movementSpeed);
        break;
    case 'a': case 'A': // W LEWO
        updateCenter(-rightX * movementSpeed, 0.0f, -rightZ * movementSpeed);
        break;
    case 'd': case 'D': // W PRAWO
        updateCenter(rightX * movementSpeed, 0.0f, rightZ * movementSpeed);
        break;
    case 'z': case 'Z': // W GÓRÊ
        updateCenter(0.0f, movementSpeed, 0.0f);
        break;
    case 'x': case 'X': // W DÓ£
        updateCenter(0.0f, -movementSpeed, 0.0f);
        break;
    default:
        return;
    }
}