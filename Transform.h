#pragma once
#include <DirectXMath.h>

class Transform
{
	public:
		Transform();

		// Transformations
		void MoveAbsolute(float x, float y, float z);
		void MoveRelative(float x, float y, float z);
		void Rotate(float p, float y, float r);
		void Scale(float x, float y, float z);

		// Setters
		void SetPosition(float x, float y, float z);
		void SetRotation(float p, float y, float r);
		void SetScale(float x, float y, float z);

		// Getters
		DirectX::XMFLOAT3 GetPosition();
		DirectX::XMFLOAT3 GetPitchYawRoll();
		DirectX::XMFLOAT3 GetScale();
		DirectX::XMFLOAT4X4 GetWorldMatrix();
		DirectX::XMFLOAT4X4 GetWorldInverseTranposeMatrix();
		DirectX::XMFLOAT3 GetRight();
		DirectX::XMFLOAT3 GetUp();
		DirectX::XMFLOAT3 GetForward();

		// Helper
		void UpdateDirections();

	private:
		// Raw transformation data
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 pitchYawRoll;
		DirectX::XMFLOAT3 scale;

		// Direction Vectors
		DirectX::XMFLOAT3 right;
		DirectX::XMFLOAT3 up;
		DirectX::XMFLOAT3 forward;

		// Finalized Matrix
		DirectX::XMFLOAT4X4 worldMatrix;
		DirectX::XMFLOAT4X4 worldInverseTranspose;

		// Matrix Updated
		bool matrixDirty;

		// Helper Function
		void CreateWorldMatrices();
};

