#pragma once
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
#include <memory>
class Entity
{
	public:
		Entity(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);

		Transform* GetTransform();
		std::shared_ptr<Mesh> GetMesh();
		std::shared_ptr<Material> GetMaterial();

		// Setters
		void SetTransform(Transform _transform);
		void SetMesh(std::shared_ptr<Mesh> _mesh);
		void SetMaterial(std::shared_ptr<Material> _material);

	private:
		Transform transform;
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
};

