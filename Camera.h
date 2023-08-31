#pragma once

#include "Transform.h"
#include <DirectXMath.h>

class Camera
{
	public:
		Camera(float x, float y, float z, float _aspectRatio, float _fieldOfView, float _nearPlane, float _farPlane);
		~Camera();

		// Update Methods
		void Update(float dt);
		void UpdateViewMatrix();
		void UpdateProjectionMatrix(float _aspectRatio);

		// Getters
		Transform* GetTransform();
		DirectX::XMFLOAT4X4 GetViewMatrix();
		DirectX::XMFLOAT4X4 GetProjectionMatrix();

	private:
		// Camera Matrices
		DirectX::XMFLOAT4X4 viewMatrix;
		DirectX::XMFLOAT4X4 projectionMatrix;

		// Transformation info
		Transform transform;

		// Custimizable Variables
		float aspectRatio;
		float fieldOfView;
		float nearPlane;
		float farPlane;
};

