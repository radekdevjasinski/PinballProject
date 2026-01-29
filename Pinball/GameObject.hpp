#pragma once
#include <PxPhysicsAPI.h>
#include <GL/freeglut.h>
#include "ModelManager.hpp"

using namespace physx;

enum class ObjectType {
    MODEL,  
    SPHERE, 
    PLANE,
    FLIPPER
};

class GameObject {
public:
    ObjectType type;
    ModelManager* model = nullptr;

    PxRigidActor* actor = nullptr;

    PxVec3 position;
    PxQuat rotation;
    PxVec3 scale;

    float radius = 0.5f;

    GameObject(ObjectType t = ObjectType::MODEL)
        : type(t), position(0, 0, 0), rotation(PxIdentity), scale(1, 1, 1) {
    }

    void setScale(float x, float y, float z) {
        scale = PxVec3(x, y, z);
    }

    void update() {
        if (actor && type == ObjectType::SPHERE) {
            PxTransform t = actor->getGlobalPose();
            position = t.p;
            rotation = t.q;
        }
    }

    void draw() {
        glPushMatrix();

        glTranslatef(position.x, position.y, position.z);

        PxVec3 axis;
        PxReal angle;
        rotation.toRadiansAndUnitAxis(angle, axis);
        glRotatef(angle * 180.0f / 3.14159f, axis.x, axis.y, axis.z);

        glScalef(scale.x, scale.y, scale.z);

        if (type == ObjectType::MODEL && model) {
            //model->drawTable();
        }
        else if (type == ObjectType::SPHERE) {
            glDisable(GL_COLOR_MATERIAL);

            GLfloat mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
            GLfloat mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
            GLfloat mat_specular[] = { 0.774f, 0.774f, 0.774f, 1.0f };
            GLfloat mat_shininess[] = { 76.8f };

            glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

            glutSolidSphere(radius, 32, 32); 

            glEnable(GL_COLOR_MATERIAL);
            GLfloat no_mat[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            GLfloat default_shininess[] = { 0.0f };
            glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
            glMaterialfv(GL_FRONT, GL_SHININESS, default_shininess);
        }
        else if (type == ObjectType::PLANE) {

        }

        glPopMatrix();
    }
};