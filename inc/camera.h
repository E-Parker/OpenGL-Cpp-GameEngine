#pragma once

#include "asset.h"

// Forward Declarations:
struct Camera;
typedef void (*CameraUpdateFunction)(Camera*, const double, const double);
typedef void (*CameraInitializeFunction)(Camera*);


void DefaultCameraInit(Camera* camera);
void NoClipCameraUpdate(Camera* camera, const double deltaTime, const double aspectRatio);


typedef struct Camera {
    
    ASSET_BODY(ObjectType::Camera)
    Quaternion Rotation;
    Matrix ViewMatrix;
    
    float MoveSpeed = 1.0f;
    float Acceleration = 0.4f;
    float Fov = 60.0f;
    float NearClip = 0.001f;
    float FarClip = 1024.0f;
    float Sensitivity = 1.0f;

    // function pointers:
    CameraUpdateFunction Update = NoClipCameraUpdate;
    CameraInitializeFunction Init = DefaultCameraInit;
    
    Camera();
    Camera(CameraUpdateFunction update);
    Camera(CameraUpdateFunction update, CameraInitializeFunction init);
    Camera(CameraUpdateFunction update, float moveSpeed, float acceleration, float sensitivity, float fov, float nearCLip, float farClip);
    Camera(CameraUpdateFunction update, CameraInitializeFunction init, float moveSpeed, float acceleration, float sensitivity, float fov, float nearCLip, float farClip);
    ~Camera();

} Camera;

