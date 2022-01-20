#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>

struct SimpleVertex
{
	float pos[3];
	float norm[3];
	float uv[2];

	SimpleVertex(const std::array<float, 3>& position, const std::array<float, 3>& normal, const std::array<float, 2>& uvCoords)
	{
		for (int i = 0; i < 3; i++)
		{
			pos[i] = position[i];
			norm[i] = normal[i];
		}
		for (int i = 0; i < 2; i++)
		{
			uv[i] = uvCoords[i];
		}
	}
};

struct SimplePointLight
{
	float pos[3];
	float padding;
	float col[3];
	float padding2;

	SimplePointLight(const std::array<float, 3>& position, const std::array<float, 3>& color)
	{
		for (int i = 0; i < 3; i++)
		{
			pos[i] = position[i];
			col[i] = color[i];
		}
		padding = 42;
		padding2 = 42;
	}
};

struct SimpleMaterial
{
	float ka;
	float kd;
	float ks;
	int shiny;

	SimpleMaterial(float ambientK, float diffuseK, float specularK, int shinyness)
	{
		ka = ambientK;
		kd = diffuseK;
		ks = specularK;
		shiny = shinyness;
	}
};

bool SetupPipeline(ID3D11Device* device, ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer, ID3D11Buffer*& transformBuffer,
	DirectX::XMMATRIX& worldMatrix, DirectX::XMMATRIX& viewProjMatrix, ID3D11VertexShader*& vShader, ID3D11PixelShader*& pShader,
	ID3D11InputLayout*& inputLayout, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& resourceView, ID3D11SamplerState*& sampler, 
	ID3D11Buffer*& lightBuffer, ID3D11Buffer*& materialBuffer);