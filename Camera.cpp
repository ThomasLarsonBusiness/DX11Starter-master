#include "Camera.h"
#include "Input.h"

using namespace DirectX;

/// <summary>
/// Constructor for the Camera
/// </summary>
/// <param name="x">Starting X</param>
/// <param name="y">Starting Y</param>
/// <param name="z">Starting Z</param>
/// <param name="aspectRatio">Screen Aspect Ratio</param>
Camera::Camera(float x, float y, float z, float _aspectRatio, float _fieldOfView, float _nearPlane, float _farPlane) : transform()
{
    // Set up transform and matrices
    transform.SetPosition(x, y, z);
    aspectRatio = _aspectRatio;
    fieldOfView = _fieldOfView;
    nearPlane = _nearPlane;
    farPlane = _farPlane;
    UpdateViewMatrix();
    UpdateProjectionMatrix(_aspectRatio);
}

/// <summary>
/// Camera Destructor (Currently Does Nothing)
/// </summary>
Camera::~Camera()
{
}

/// <summary>
/// Manages keyboard input and updates the matrices
/// </summary>
/// <param name="dt">Delta Time</param>
void Camera::Update(float dt)
{

    // Take in user input
    Input& input = Input::GetInstance();    // Gets Singleton Instance

    // Camera's Speed
    float speed = dt * 5.0f;

    // Speed Modifiers
    if (input.KeyDown(VK_SHIFT)) { speed *= 5.0f; }
    if (input.KeyDown(VK_CONTROL)) { speed *= 0.1f; }

    // Move based on keyboard input
    if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
    if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
    if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0, 0); }
    if (input.KeyDown('D')) { transform.MoveRelative(speed, 0, 0); }
    if (input.KeyDown('X')) { transform.MoveAbsolute(0, -speed, 0); }
    if (input.KeyDown(' ')) { transform.MoveAbsolute(0, speed, 0); }

    // Mouse movement on left click
    if (input.MouseLeftDown()) 
    {
        // Calculate the cursor change
        float lookSpeed = dt * 2.0f;
        float xDiff = input.GetMouseXDelta() * lookSpeed;
        float yDiff = input.GetMouseYDelta() * lookSpeed;

        // Rotate the transform
        transform.Rotate(yDiff, xDiff, 0);
    }

    // Update the View Matrix
    UpdateViewMatrix();

    // Update field of view
    if (input.KeyDown(VK_OEM_PLUS)) { fieldOfView += dt; }
    if (input.KeyDown(VK_OEM_MINUS)) { fieldOfView -= dt; }
    UpdateProjectionMatrix(aspectRatio);
}

/// <summary>
/// Updates the view matrix
/// </summary>
void Camera::UpdateViewMatrix()
{
    // Bring in necessary information from transform
    XMFLOAT3 pos = transform.GetPosition();
    XMFLOAT3 forward = transform.GetForward();
    XMFLOAT3 up = transform.GetUp();

    // Update and store the view matrix
    XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&forward), XMLoadFloat3(&up));
    XMStoreFloat4x4(&viewMatrix, view);
}

/// <summary>
/// Updates the projection matrix
/// </summary>
/// <param name="aspectRatio">The aspect ratio of the screen</param>
void Camera::UpdateProjectionMatrix(float _aspectRatio)
{
    // Updates Aspect Ratio
    aspectRatio = _aspectRatio;

    // Creates a projection matrix with a 45 degree FOV
    XMMATRIX proj = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearPlane, farPlane);
    XMStoreFloat4x4(&projectionMatrix, proj);
}

// Getters
Transform* Camera::GetTransform() { return &transform; }
DirectX::XMFLOAT4X4 Camera::GetViewMatrix() { return viewMatrix; }
DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix() { return projectionMatrix; }
