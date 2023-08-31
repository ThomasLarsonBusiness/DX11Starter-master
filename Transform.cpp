#include "Transform.h"

// Reduces code, and allows for operator overloads
using namespace DirectX;

/// <summary>
/// Constructor
/// </summary>
Transform::Transform()
{
	// Set up our initial values
	SetPosition(0, 0, 0);
	SetRotation(0, 0, 0);
	SetScale(1, 1, 1);

	// Creates the initial matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	
	// Set matrix bool
	matrixDirty = false;
}

/// <summary>
/// Moves the object along the world X Y Z
/// </summary>
void Transform::MoveAbsolute(float x, float y, float z)
{
	// Using the math library
	XMVECTOR pos = XMLoadFloat3(&position);
	XMVECTOR offset = XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, pos + offset);

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Moves the object along the objects local X Y Z
/// </summary>
void Transform::MoveRelative(float x, float y, float z)
{
	// Initial move vector
	XMVECTOR moveVec = XMVectorSet(x, y, z, 0);

	// Rotate move vector by transform's orientaion
	XMVECTOR rotVec = XMVector3Rotate(moveVec, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));

	// Add the rotated move vector to position and overwrite
	XMVECTOR newPos = XMLoadFloat3(&position) + rotVec;
	XMStoreFloat3(&position, newPos);

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Rotates the object by a given euler angle
/// </summary>
void Transform::Rotate(float p, float y, float r)
{
	// Using the math library
	XMVECTOR pyr = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR rot = XMVectorSet(p, y, r, 0);
	XMStoreFloat3(&pitchYawRoll, pyr + rot);

	// Update Directional Vectors
	UpdateDirections();

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Multiplies the scale by the incoming scale
/// </summary>
void Transform::Scale(float x, float y, float z)
{
	// Using the math library
	XMVECTOR scl = XMLoadFloat3(&scale);
	XMVECTOR change = XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&scale, XMVectorMultiply(scl, change));

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Sets the objects position
/// </summary>
void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Sets the objects rotation using Euler angles
/// </summary>
void Transform::SetRotation(float p, float y, float r)
{
	pitchYawRoll = XMFLOAT3(p, y, r);

	// Update Directional Vectors
	UpdateDirections();

	// Updated matrix dirty
	matrixDirty = true;
}

/// <summary>
/// Sets the objects scale
/// </summary>
void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);

	// Updated matrix dirty
	matrixDirty = true;
}

// Transform Component Getters
DirectX::XMFLOAT3 Transform::GetPosition(){ return position; }
DirectX::XMFLOAT3 Transform::GetPitchYawRoll(){ return pitchYawRoll; }
DirectX::XMFLOAT3 Transform::GetScale(){ return scale; }

/// <summary>
/// Getter that updates the world matrix if necessary before returning the world matrix
/// </summary>
/// <returns>The world matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Only updates the matrix if there was a change
	if (matrixDirty){ CreateWorldMatrices(); }
	
	// Returns the world matrix
	return worldMatrix;
}

/// <summary>
/// Getter that updates the world matrices if necessary before return the world inverse transpose matrix
/// </summary>
/// <returns>The world inverse transpose matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranposeMatrix()
{
	if (matrixDirty) { CreateWorldMatrices(); }
	return worldInverseTranspose;
}

// Directional Vector Getters
DirectX::XMFLOAT3 Transform::GetRight() { return forward; }
DirectX::XMFLOAT3 Transform::GetUp() { return up; }
DirectX::XMFLOAT3 Transform::GetForward() { return forward; }

/// <summary>
/// Updates the directional vectors
/// </summary>
void Transform::UpdateDirections()
{
	// Generates a rotation quaternion
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);

	// Updates the 3 directional vectors
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rot));
	XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rot));
	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rot));
}

/// <summary>
/// Helper function to create both world matrices if the matrices are dirty
/// </summary>
void Transform::CreateWorldMatrices()
{
	// Create individual transformation matrices
	XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
	XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);

	// Combine transformations matrices and store result
	XMMATRIX worldMat = scaleMatrix * rotationMatrix * translationMatrix;
	XMStoreFloat4x4(&worldMatrix, worldMat);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

	// Mark matrix as clean
	matrixDirty = false;
}
