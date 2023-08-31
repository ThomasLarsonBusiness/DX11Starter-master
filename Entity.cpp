#include "Entity.h"

/// <summary>
/// Constructor for Entity. Takes a mesh pointer and creates its own transform
/// </summary>
Entity::Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material) : transform()
{
    mesh = _mesh;
    material = _material;
}

// Getters
Transform* Entity::GetTransform() { return &transform; }
std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }
std::shared_ptr<Material> Entity::GetMaterial() { return material; }

// Setters
void Entity::SetTransform(Transform _transform) { transform = _transform; }
void Entity::SetMesh(std::shared_ptr<Mesh> _mesh) { mesh = _mesh; }
void Entity::SetMaterial(std::shared_ptr<Material> _material) { material = _material; }
