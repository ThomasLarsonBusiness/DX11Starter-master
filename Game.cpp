#include "Game.h"
#include "Vertex.h"
#include "Input.h"
//#include "WICTextureLoader.h"
#include "packages/directxtk_desktop_win10.2022.3.24.2/include/WICTextureLoader.h"
#include "packages/directxtk_desktop_win10.2022.3.24.2/include/DDSTextureLoader.h"
//#include "DDSTextureLoader.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true),			   // Show extra stats (fps) in title bar?
	vsync(false)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	// Create a Camera
	camera = std::make_shared<Camera>(0.0f, 2.0f, -20.0f, (float)width/height, XM_PIDIV4, 0.01f, 1000.0f);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Gametr
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Sets up the light
	ambientLight = DirectX::XMFLOAT3(0.0f, 0.0f, 0.05f);

	// Directinal Light 1
	directionalLight1 = {};
	directionalLight1.Type = 0;
	directionalLight1.Direction = DirectX::XMFLOAT3(1, -1, 0);
	directionalLight1.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLight1.Intensity = 0.5f;

	// Directional Light 2
	directionalLight2 = {};
	directionalLight2.Type = 0;
	directionalLight2.Direction = DirectX::XMFLOAT3(-1, -1, 0);
	directionalLight2.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLight2.Intensity = 0.5f;

	// Directional Light 3
	directionalLight3 = {};
	directionalLight3.Type = 0;
	directionalLight3.Direction = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight3.Color = DirectX::XMFLOAT3(1.0, 0.0f, 0.0f);
	directionalLight3.Intensity = 0.5f;

	// Point Light 1
	pointLight1 = {};
	pointLight1.Type = 1;
	pointLight1.Range = 4.0f;
	pointLight1.Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	pointLight1.Intensity = 3.0f;
	pointLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

	// Point Light 2
	pointLight2 = {};
	pointLight2.Type = 1;
	pointLight2.Range = 4.0f;
	pointLight2.Position = DirectX::XMFLOAT3(4.0f, 1.0f, 0.0f);
	pointLight2.Intensity = 3.0f;
	pointLight2.Color = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Loads in the simple shaders
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, GetFullPathTo_Wide(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"PixelShader.cso").c_str());
	customPS = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"CustomPS.cso").c_str());
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context, GetFullPathTo_Wide(L"SkyVertexShader.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"SkyPixelShader.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices1[] =
	{
		{ XMFLOAT3(-0.7f, +0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{ XMFLOAT3(-0.5f, +0.0f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{ XMFLOAT3(-0.9f, +0.0f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
	};

	// Set up the indices, which tell us which vertices to use and in which order
	// - This is somewhat redundant for just 3 vertices (it's a simple example)
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices1[] = { 0, 1, 2 };
	std::shared_ptr<Mesh> mesh1 = std::make_shared<Mesh>(vertices1, sizeof(vertices1)/sizeof(Vertex), indices1, sizeof(indices1)/sizeof(unsigned int), device, context);
	meshes.push_back(mesh1);

	// Rectangle
	Vertex vertices2[] =
	{
		{ XMFLOAT3(+0.4f, +0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{ XMFLOAT3(+0.7f, +0.2f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
		{ XMFLOAT3(+0.4f, +0.2f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) }
	};

	unsigned int indices2[] = { 0, 1, 2, 2, 3, 0};
	std::shared_ptr<Mesh> mesh2 = std::make_shared<Mesh>(vertices2, sizeof(vertices2) / sizeof(Vertex), indices2, sizeof(indices2) / sizeof(unsigned int), device, context);
	meshes.push_back(mesh2);

	// Cursed Shape
	Vertex vertices3[] =
	{
		{ XMFLOAT3(+0.0f, +0.6f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.4f, +0.0f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.0f, +0.0f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.4f, +0.0f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.2f, -0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.2f, -0.7f, +0.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};

	unsigned int indices3[] = { 1, 0, 3, 1, 2, 4, 2, 3, 5};
	std::shared_ptr<Mesh> mesh3 = std::make_shared<Mesh>(vertices3, sizeof(vertices3)/sizeof(Vertex), indices3, sizeof(indices3)/sizeof(unsigned int), device, context);
	meshes.push_back(mesh3);

	// Creates a sampler state
	D3D11_SAMPLER_DESC ssd = {};
	ssd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ssd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ssd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ssd.Filter = D3D11_FILTER_ANISOTROPIC;
	ssd.MaxAnisotropy = 16;
	ssd.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ssd, samplerState.GetAddressOf());

	// Loads in textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cushion.png").c_str(), nullptr, texture1.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/cushion_normals.png").c_str(), nullptr, normal1.GetAddressOf());

	// Loads in PBR textures
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(), nullptr, albedo1.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(), nullptr, normals1.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(), nullptr, roughness1.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/scratched_metalness.png").c_str(), nullptr, metalness1.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(), nullptr, albedo2.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_normals.png").c_str(), nullptr, normals2.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(), nullptr, roughness2.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/floor_metalness.png").c_str(), nullptr, metalness2.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), nullptr, albedo3.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), nullptr, normals3.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), nullptr, roughness3.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_metalness.png").c_str(), nullptr, metalness3.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/rough_albedo.png").c_str(), nullptr, albedo4.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/rough_normals.png").c_str(), nullptr, normals4.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/rough_roughness.png").c_str(), nullptr, roughness4.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/rough_metalness.png").c_str(), nullptr, metalness4.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), nullptr, albedo5.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), nullptr, normals5.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), nullptr, roughness5.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_metalness.png").c_str(), nullptr, metalness5.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), nullptr, albedo6.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), nullptr, normals6.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(), nullptr, roughness6.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_metalness.png").c_str(), nullptr, metalness6.GetAddressOf());

	// Creates Materials
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(0.0f, 0.0f)));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(0.0f, 0.2f)));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(1.0f, 1.0f)));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(0.5f, 0.5f)));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(0.0f, 0.05f)));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vertexShader, pixelShader, 0.5f, 1.0f, XMFLOAT2(0.0f, 0.00f)));

	// Adds the texture to the materials
	/*
	materials[0]->AddTextureSRV("SurfaceTexture", texture1);
	materials[0]->AddTextureSRV("NormalMap", normal1);
	materials[0]->AddSampler("BasicSampler", samplerState);

	materials[1]->AddTextureSRV("SurfaceTexture", texture1);
	materials[1]->AddTextureSRV("NormalMap", normal1);
	materials[1]->AddSampler("BasicSampler", samplerState);

	materials[2]->AddTextureSRV("SurfaceTexture", texture1);
	materials[2]->AddTextureSRV("NormalMap", normal1);
	materials[2]->AddSampler("BasicSampler", samplerState);

	materials[3]->AddTextureSRV("SurfaceTexture", texture1);
	materials[3]->AddTextureSRV("NormalMap", normal1);
	materials[3]->AddSampler("BasicSampler", samplerState);

	materials[4]->AddTextureSRV("SurfaceTexture", texture1);
	materials[4]->AddTextureSRV("NormalMap", normal1);
	materials[4]->AddSampler("BasicSampler", samplerState);
	*/

	materials[0]->AddTextureSRV("AlbedoTexture", albedo1);
	materials[0]->AddTextureSRV("NormalMap", normals1);
	materials[0]->AddTextureSRV("RoughnessMap", roughness1);
	materials[0]->AddTextureSRV("MetalnessMap", metalness1);
	materials[0]->AddSampler("BasicSampler", samplerState);

	materials[1]->AddTextureSRV("AlbedoTexture", albedo2);
	materials[1]->AddTextureSRV("NormalMap", normals2);
	materials[1]->AddTextureSRV("RoughnessMap", roughness2);
	materials[1]->AddTextureSRV("MetalnessMap", metalness2);
	materials[1]->AddSampler("BasicSampler", samplerState);

	materials[2]->AddTextureSRV("AlbedoTexture", albedo3);
	materials[2]->AddTextureSRV("NormalMap", normals3);
	materials[2]->AddTextureSRV("RoughnessMap", roughness3);
	materials[2]->AddTextureSRV("MetalnessMap", metalness3);
	materials[2]->AddSampler("BasicSampler", samplerState);

	materials[3]->AddTextureSRV("AlbedoTexture", albedo4);
	materials[3]->AddTextureSRV("NormalMap", normals4);
	materials[3]->AddTextureSRV("RoughnessMap", roughness4);
	materials[3]->AddTextureSRV("MetalnessMap", metalness4);
	materials[3]->AddSampler("BasicSampler", samplerState);

	materials[4]->AddTextureSRV("AlbedoTexture", albedo5);
	materials[4]->AddTextureSRV("NormalMap", normals5);
	materials[4]->AddTextureSRV("RoughnessMap", roughness5);
	materials[4]->AddTextureSRV("MetalnessMap", metalness5);
	materials[4]->AddSampler("BasicSampler", samplerState);

	materials[5]->AddTextureSRV("AlbedoTexture", albedo6);
	materials[5]->AddTextureSRV("NormalMap", normals6);
	materials[5]->AddTextureSRV("RoughnessMap", roughness6);
	materials[5]->AddTextureSRV("MetalnessMap", metalness6);
	materials[5]->AddSampler("BasicSampler", samplerState);

	// Creates mesh from 3D object
	std::shared_ptr<Mesh> mesh4 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device);
	meshes.push_back(mesh4);
	std::shared_ptr<Mesh> mesh5 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device);
	meshes.push_back(mesh5);
	std::shared_ptr<Mesh> mesh6 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device);
	meshes.push_back(mesh6);
	std::shared_ptr<Mesh> mesh7 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad.obj").c_str(), device);
	meshes.push_back(mesh7);
	std::shared_ptr<Mesh> mesh8 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/quad_double_sided.obj").c_str(), device);
	meshes.push_back(mesh8);
	std::shared_ptr<Mesh> mesh9 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/torus.obj").c_str(), device);
	meshes.push_back(mesh9);
	std::shared_ptr<Mesh> mesh10 = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device);
	meshes.push_back(mesh10);

	entities.push_back(std::make_shared<Entity>(meshes[3], materials[0]));
	entities.push_back(std::make_shared<Entity>(meshes[4], materials[1]));
	entities.push_back(std::make_shared<Entity>(meshes[5], materials[2]));
	entities.push_back(std::make_shared<Entity>(meshes[6], materials[3]));
	entities.push_back(std::make_shared<Entity>(meshes[7], materials[4]));
	entities.push_back(std::make_shared<Entity>(meshes[8], materials[5]));

	// Creates the skybox texture
	CreateDDSTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/sunnyCubeMap.dds").c_str(), nullptr, skyboxTexture.GetAddressOf());
	skybox = std::make_shared<Sky>(meshes[9], samplerState, device, skyVertexShader, skyPixelShader, skyboxTexture);
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();


	camera->UpdateProjectionMatrix((float)width/height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	// Updates the test transform
	/*
	transform.SetPosition(sin(totalTime), 0, 0);
	float scale = cos(totalTime) * 0.5f + 0.5f;
	transform.SetScale(scale, scale, scale);
	transform.Rotate(0, 0, deltaTime * 0.1f);
	*/

	//entities[0]->GetTransform()->SetPosition(sin(totalTime), 0, 0);
	//entities[1]->GetTransform()->SetPosition(-sin(totalTime), 0, 0);
	//float scale = cos(totalTime) * 0.5f + 0.5f;
	//entities[2]->GetTransform()->SetScale(scale, scale, scale);
	//entities[3]->GetTransform()->MoveAbsolute(-0.5f * deltaTime, -0.5f * deltaTime, 0);
	//entities[4]->GetTransform()->Rotate(0, 0, deltaTime * 0.3f);

	entities[0]->GetTransform()->SetPosition(-10.0f, 0.0f, 0.0f);
	entities[1]->GetTransform()->SetPosition(-6.0f, 0.0f, 0.0f);
	entities[2]->GetTransform()->SetPosition(-2.0f, 0.0f, 0.0f);
	entities[3]->GetTransform()->SetPosition(2.0f, 0.0f, 0.0f);
	entities[4]->GetTransform()->SetPosition(6.0f, 0.0f, 0.0f);
	entities[5]->GetTransform()->SetPosition(10.0f, 0.0f, 0.0f);

	/*
	std::shared_ptr<SimplePixelShader> ps = entities[0]->GetMaterial()->GetPixelShader();
	ps->SetFloat("totalTime", totalTime);
	ps = entities[5]->GetMaterial()->GetPixelShader();
	ps->SetFloat("totalTime", totalTime);
	*/

	camera->Update(deltaTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	//context->VSSetShader(vertexShader.Get(), 0, 0);
	//context->PSSetShader(pixelShader.Get(), 0, 0);


	// Ensure the pipeline knows how to interpret the data (numbers)
	// from the vertex buffer.  
	// - If all of your 3D models use the exact same vertex layout,
	//    this could simply be done once in Init()
	// - However, this isn't always the case (but might be for this course)
	//context->IASetInputLayout(inputLayout.Get());

	// Loop through all entities and draw their information
	for (int i = 0; i < entities.size(); i++)
	{
		// Set the current shaders
		entities[i]->GetMaterial()->GetVertexShader()->SetShader();
		entities[i]->GetMaterial()->GetPixelShader()->SetShader();

		// Defines the Vertex Shader data
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();
		vs->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorldMatrix());
		vs->SetMatrix4x4("worldInvTranspose", entities[i]->GetTransform()->GetWorldInverseTranposeMatrix());
		vs->SetMatrix4x4("view", camera->GetViewMatrix());
		vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
		vs->CopyAllBufferData();

		// Defines the Pixel Shader data
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		ps->SetFloat4("colorTint", entities[i]->GetMaterial()->GetColorTint());
		ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
		ps->SetFloat("roughness", entities[i]->GetMaterial()->GetRoughness());
		ps->SetFloat3("cameraPos", camera->GetTransform()->GetPosition());
		ps->SetFloat3("ambientLight", ambientLight);
		ps->SetFloat("uvScale", entities[i]->GetMaterial()->GetUvScale());
		ps->SetFloat2("uvOffset", entities[i]->GetMaterial()->GetUvOffset());
		ps->SetData("directionalLight1", &directionalLight1, sizeof(Light));
		ps->SetData("directionalLight2", &directionalLight2, sizeof(Light));
		ps->SetData("directionalLight3", &directionalLight3, sizeof(Light));
		ps->SetData("pointLight1", &pointLight1, sizeof(Light));
		ps->SetData("pointLight2", &pointLight2, sizeof(Light));
		entities[i]->GetMaterial()->SetMaps();
		ps->CopyAllBufferData();

		// Sets stride and offset
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		// Sets the Vertex and Index Buffers
		context->IASetVertexBuffers(0, 1, entities[i]->GetMesh()->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(entities[i]->GetMesh()->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		// Draws the mesh to the screen
		context->DrawIndexed(
			entities[i]->GetMesh()->GetIndexCount(),
			0,
			0);
	}

	// Draws the skybox
	skybox->Draw(context, camera);
	

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(vsync ? 1 : 0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}