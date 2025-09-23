#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>

#include "glUtilities.h"
#include "vectorMath.h"
#include "camera.h"

#define mat4_print(m) printf("%f, %f, %f, %f, \n%f, %f, %f, %f, \n%f, %f, %f, %f, \n%f, %f, %f, %f \n\n", m[0], m[4], m[8], m[12], m[1], m[5], m[9], m[13], m[2], m[6], m[10], m[14], m[3], m[7], m[11], m[15])

Camera::Camera() {}

Camera::Camera(CameraUpdateFunction update) {
    Update = update;
    Init = DefaultCameraInit;
    Init(this);
}

Camera::Camera(CameraUpdateFunction update, float MoveSpeed, float acceleration, float sensitivity, float fov, float nearClip = 0.01f, float farClip = 1024.0f) : MoveSpeed(MoveSpeed), Acceleration(acceleration), Sensitivity(sensitivity), Fov(fov), NearClip(nearClip), FarClip(farClip) {
    Update = update;
    Init = DefaultCameraInit;
    Init(this);
}

Camera::Camera(CameraUpdateFunction update, CameraInitializeFunction init) {
    Update = update;
    Init = init;
    Init(this);
}

Camera::Camera(CameraUpdateFunction update, CameraInitializeFunction init, float MoveSpeed, float acceleration, float sensitivity, float fov, float nearClip = 0.01f, float farClip = 1024.0f) : MoveSpeed(MoveSpeed), Acceleration(acceleration), Sensitivity(sensitivity), Fov(fov), NearClip(nearClip), FarClip(farClip) {
    Update = update;
    Init = init;
    Init(this);
}

Camera::~Camera() {}

void DefaultCameraInit(Camera* camera) {
    SetCaptureCursor(true);
    camera->Transform = MatrixIdentity();
    camera->ViewMatrix = MatrixIdentity();
    camera->Rotation = QuaternionIdentity();
}


void NoClipCameraUpdate(Camera* camera, const double deltaTime, const double ratio) {
    /* Update function for a camera with noclip. */

    Vector3 desiredMovement {0.0f, 0.0f, 0.0f};

    // Handle input:
    
    if(IsKeyDown(GLFW_KEY_W)) {
        desiredMovement += V3_FORWARD;
    }

    if(IsKeyDown(GLFW_KEY_S)) {
        desiredMovement -= V3_FORWARD;
    }

    if(IsKeyDown(GLFW_KEY_D)) {
        desiredMovement -= V3_RIGHT;
    }

    if(IsKeyDown(GLFW_KEY_A)) {
        desiredMovement += V3_RIGHT;
    }

    if(IsKeyDown(GLFW_KEY_SPACE)) {
        desiredMovement -= V3_UP;
    }

    if(IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        desiredMovement += V3_UP;
    }
    
    // Get the cursor position and generate a rotation matrix from it.
    double CursorPositionX, CursorPositionY;
    GetCursorPositionDelta(&CursorPositionX, &CursorPositionY);
    
    camera->Rotation = camera->Rotation * FromAxisAngle(V3_RIGHT, camera->Sensitivity * PI * (float)CursorPositionY);
    camera->Rotation = FromAxisAngle(V3_UP, camera->Sensitivity * PI * (float)CursorPositionX) * camera->Rotation;

    // Normalize to account for fractional losses due to floating point error.
    camera->Rotation = Normalize(camera->Rotation); 

    desiredMovement = Normalize(desiredMovement);
    desiredMovement *= (camera->MoveSpeed * (float)DeltaTime());
    desiredMovement = Rotate(desiredMovement, camera->Rotation);

    // Apply the movement matrix and rotation matrix to the camera's transform.
    camera->Transform = camera->Transform * Translate(desiredMovement.x, desiredMovement.y, desiredMovement.z);

    Vector3 Position = Translation(camera->Transform);

    // Recalculate the view matrix from the updated camera transform and rotation.
    Matrix rotationMatrix = ToMatrix(Invert(camera->Rotation));

    Matrix projection = Perspective(DEG2RAD * camera->Fov, ratio, camera->NearClip, camera->FarClip);
    //Matrix projection = Ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);

    camera->ViewMatrix = camera->Transform * rotationMatrix * projection;
   
}
