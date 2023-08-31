#pragma once

#include "Vertex.h"
#include <d3d11.h>
#include <wrl/client.h>

// Purpose is create and store the buffers for objects to be drawn to the screen
class Mesh
{
	private:
		// Fields
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
		int numIndices;

		// Methods
		void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	public:
		// Constructors
		Mesh(
			Vertex* _vertices,
			int _numVertices,
			unsigned int* _indices,
			int _numIndicies,
			Microsoft::WRL::ComPtr<ID3D11Device> _device,
			Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
		Mesh(const char* objFile, Microsoft::WRL::ComPtr<ID3D11Device> _device);
		~Mesh(); // Deconstructor

		// Functions
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
		int GetIndexCount();
};