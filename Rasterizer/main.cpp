#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <chrono>

#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

void Render(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsView, D3D11_VIEWPORT& viewport,
			ID3D11VertexShader* vShader, ID3D11PixelShader* pShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* vertexBuffer, 
			ID3D11Buffer* indexBuffer, ID3D11Buffer* transformBuffer, ID3D11ShaderResourceView* resourceView, ID3D11SamplerState* sampler, 
			ID3D11Buffer* lightBuffer, ID3D11Buffer* materialBuffer)
{
	float clearColour[4] = { 0, 0, 0, 0 };
	ID3D11Buffer* vsBuffers[2] = { transformBuffer, lightBuffer };
	ID3D11Buffer* psBuffers[2] = { lightBuffer, materialBuffer};

	immediateContext->ClearRenderTargetView(rtv, clearColour);
	immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	immediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	immediateContext->IASetInputLayout(inputLayout);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	immediateContext->VSSetConstantBuffers(0, 2, vsBuffers);
	immediateContext->VSSetShader(vShader, nullptr, 0);

	immediateContext->RSSetViewports(1, &viewport);

	immediateContext->PSSetShaderResources(0, 1, &resourceView);
	immediateContext->PSSetSamplers(0, 1, &sampler);
	immediateContext->PSSetConstantBuffers(0, 2, psBuffers);
	immediateContext->PSSetShader(pShader, nullptr, 0);
	
	immediateContext->OMSetRenderTargets(1, &rtv, dsView);

	immediateContext->DrawIndexed(6, 0, 0);
}

void Rotate(double fps, std::chrono::time_point<std::chrono::steady_clock>& previous, std::chrono::steady_clock& timer, ID3D11DeviceContext* immediateContext, ID3D11Buffer* transformBuffer, DirectX::XMMATRIX& worldMatrix, DirectX::XMMATRIX& rotationStep, DirectX::XMMATRIX& viewProjMatrix)
{
	std::chrono::time_point<std::chrono::steady_clock> now = timer.now();

	std::chrono::duration<double> deltaTime = now - previous;

	if (deltaTime.count() < 1.0 / fps)
	{
		return;
	}
	previous = now;

	worldMatrix *= rotationStep;

	DirectX::XMMATRIX worldTranspose = DirectX::XMMatrixTranspose(worldMatrix);
	DirectX::XMMATRIX viewProjTranspose = DirectX::XMMatrixTranspose(viewProjMatrix);

	DirectX::XMFLOAT4X4 matrices[2];
	DirectX::XMStoreFloat4x4(&matrices[0], worldTranspose);
	DirectX::XMStoreFloat4x4(&matrices[1], viewProjTranspose);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	immediateContext->Map(transformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, matrices, sizeof(matrices));
	immediateContext->Unmap(transformBuffer, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* dsTexture;
	ID3D11DepthStencilView* dsView;
	D3D11_VIEWPORT viewport;
	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* transformBuffer;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* resourceView;
	ID3D11SamplerState* sampler;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* materialBuffer;

	DirectX::XMMATRIX worldTranslation = DirectX::XMMatrixTranslation(0.0f, 0.0f, -1.0f);
	DirectX::XMMATRIX worldStartRotation = DirectX::XMMatrixRotationY(0.0f);
	DirectX::XMMATRIX worldMatrix = worldTranslation * worldStartRotation;
	DirectX::XMMATRIX worldStepRotation = DirectX::XMMatrixRotationY(0.01f);

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.5f, 1024.0f / 576.0f, 0.1f, 4.0f);
	DirectX::XMMATRIX viewProjMatrix = viewMatrix * projectionMatrix;

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, immediateContext, swapChain, rtv, dsTexture, dsView, viewport))
	{
		std::cerr << "Failed setting up Direct3D11!" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, vertexBuffer, indexBuffer, transformBuffer, worldMatrix, viewProjMatrix, vShader, pShader, inputLayout, texture, resourceView, sampler, lightBuffer, materialBuffer))
	{
		std::cerr << "Failed setting up pipeline!" << std::endl;
		return -1;
	}

	std::chrono::steady_clock timer;
	std::chrono::time_point<std::chrono::steady_clock> previous = timer.now();
	
	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Render(immediateContext, rtv, dsView, viewport, vShader, pShader, inputLayout, vertexBuffer, indexBuffer, transformBuffer, resourceView, sampler, lightBuffer, materialBuffer);
		swapChain->Present(0, 0);
		Rotate(60, previous, timer, immediateContext, transformBuffer, worldMatrix, worldStepRotation, viewProjMatrix);
	}

	materialBuffer->Release();
	lightBuffer->Release();
	sampler->Release();
	resourceView->Release();
	texture->Release();
	transformBuffer->Release();
	indexBuffer->Release();
	vertexBuffer->Release();
	inputLayout->Release();
	pShader->Release();
	vShader->Release();
	dsView->Release();
	dsTexture->Release();
	rtv->Release();
	swapChain->Release();
	immediateContext->Release();
	device->Release();

	return 0;
}