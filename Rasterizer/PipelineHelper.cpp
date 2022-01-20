#include "PipelineHelper.h"
#include <string>
#include <fstream>
#include <iostream>
#include <DirectXMath.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool LoadShaders(ID3D11Device* device, ID3D11VertexShader*& vShader, ID3D11PixelShader*& pShader, std::string& vShaderByteCode)
{
	std::string shaderData;
	std::ifstream reader;
	reader.open("VertexShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cerr << "Could not open vertex shader file!" << std::endl;
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());

	if (FAILED(device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &vShader)))
	{
		std::cerr << "Failed to create vertex shader!" << std::endl;
		return false;
	}

	vShaderByteCode = shaderData;


	shaderData.clear();
	reader.close();
	reader.open("PixelShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cerr << "Could not open pixel shader file!" << std::endl;
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>());

	if (FAILED(device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &pShader)))
	{
		std::cerr << "Failed to create pixel shader!" << std::endl;
		return false;
	}

	return true;
}

bool CreateInputLayout(ID3D11Device* device, ID3D11InputLayout*& inputLayout, const std::string& vShaderByteCode)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[3] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT hr = device->CreateInputLayout(inputDesc, 3, vShaderByteCode.c_str(), vShaderByteCode.length(), &inputLayout);

	return !FAILED(hr);
}

bool CreateTransformBuffer(ID3D11Device* device, ID3D11Buffer*& transformBuffer, DirectX::XMMATRIX& worldMatrix, DirectX::XMMATRIX& viewProjMatrix)
{
	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(worldMatrix);
	DirectX::XMMATRIX viewProjTranspose = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 matrices[2];
	DirectX::XMStoreFloat4x4(&matrices[0], worldTranspose);
	DirectX::XMStoreFloat4x4(&matrices[1], viewProjTranspose);

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = sizeof(matrices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = matrices;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &transformBuffer);
	return !FAILED(hr);
}

bool CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer*& vertexBuffer)
{
	SimpleVertex quad[4] =
	{
		{ {0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1.0f, 0.0f} },
		{ {0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1.0f, 1.0f} },
		{ {-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0.0f, 1.0f} },
		{ {-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0.0f, 0.0f} }
	};

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = sizeof(quad);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = quad;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &vertexBuffer);
	return !FAILED(hr);
}

bool CreateIndexBuffer(ID3D11Device* device, ID3D11Buffer*& indexBuffer)
{
	UINT indicies[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = sizeof(indicies);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = indicies;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &indexBuffer);
	return !FAILED(hr);
}

bool CreateTexture(ID3D11Device* device, ID3D11Texture2D*& texture)
{
	int x, y, n;
	unsigned char* image = stbi_load("butiful.PNG", &x, &y, &n, 0);

	D3D11_TEXTURE2D_DESC textureDesc;
	
	textureDesc.Width = x;
	textureDesc.Height = y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA data;
	
	data.pSysMem = image;
	data.SysMemPitch = n * x;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateTexture2D(&textureDesc, &data, &texture);
	stbi_image_free(image);
	return !FAILED(hr);
}

bool CreateResourceView(ID3D11Device* device, ID3D11Texture2D* texture, ID3D11ShaderResourceView*& resourceView)
{
	HRESULT hr = device->CreateShaderResourceView(texture, nullptr, &resourceView);
	return !FAILED(hr);
}

bool CreateSampler(ID3D11Device* device, ID3D11SamplerState*& sampler)
{
	D3D11_SAMPLER_DESC samplerDesc;
	
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;

	HRESULT hr = device->CreateSamplerState(&samplerDesc, &sampler);
	return !FAILED(hr);
}

bool CreateLightBuffer(ID3D11Device* device, ID3D11Buffer*& lightBuffer)
{
	SimplePointLight pointLight = { {1.0f, 0.3f, -2.0f}, {1.0f, 1.0f, 1.0f} };

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = sizeof(pointLight);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &pointLight;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &lightBuffer);
	return !FAILED(hr);
}

bool CreateMaterialBuffer(ID3D11Device* device, ID3D11Buffer*& materialBuffer)
{
	SimpleMaterial material = { 0.1f, 0.7f, 0.25f, 20 };

	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.ByteWidth = sizeof(material);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &material;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &data, &materialBuffer);
	return !FAILED(hr);
}

bool SetupPipeline(ID3D11Device* device, ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer, ID3D11Buffer*& transformBuffer, 
		DirectX::XMMATRIX& worldMatrix, DirectX::XMMATRIX& viewProjMatrix, ID3D11VertexShader*& vShader, ID3D11PixelShader*& pShader, 
	ID3D11InputLayout*& inputLayout, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& resourceView, ID3D11SamplerState*& sampler, 
	ID3D11Buffer*& lightBuffer, ID3D11Buffer*& materialBuffer)
{
	std::string vShaderByteCode;

	if (!LoadShaders(device, vShader, pShader, vShaderByteCode))
	{
		std::cerr << "Error loading shaders" << std::endl;
		return false;
	}

	if (!CreateInputLayout(device, inputLayout, vShaderByteCode))
	{
		std::cerr << "Error creating input layout" << std::endl;
		return false;
	}

	if (!CreateTransformBuffer(device, transformBuffer, worldMatrix, viewProjMatrix))
	{
		std::cerr << "Error creating transform buffer" << std::endl;
		return false;
	}

	if (!CreateVertexBuffer(device, vertexBuffer))
	{
		std::cerr << "Error creating vertex buffer" << std::endl;
		return false;
	}

	if (!CreateIndexBuffer(device, indexBuffer))
	{
		std::cerr << "Error creating index buffer" << std::endl;
		return false;
	}

	if (!CreateTexture(device, texture))
	{
		std::cerr << "Error creating Texture" << std::endl;
		return false;
	}

	if (!CreateResourceView(device, texture, resourceView))
	{
		std::cerr << "Error creating Shader Resource View" << std::endl;
		return false;
	}

	if (!CreateSampler(device, sampler))
	{
		std::cerr << "Error creating sampler state" << std::endl;
		return false;
	}

	if (!CreateLightBuffer(device, lightBuffer))
	{
		std::cerr << "Error creating light" << std::endl;
		return false;
	}

	if (!CreateMaterialBuffer(device, materialBuffer))
	{
		std::cerr << "Error creating material" << std::endl;
		return false;
	}

	return true;
}