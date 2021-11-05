#include "RenderToTexture.h"



RenderToTexture::RenderToTexture()
{
	mpID3D11RenderTargetView = nullptr;
	mpID3D11ShaderResourceView = nullptr;
	mpRenderTargetTexture = nullptr;
}

//create a copy contructor

RenderToTexture::RenderToTexture(const RenderToTexture& other)
{

}


RenderToTexture ::~RenderToTexture()
{
}

HRESULT RenderToTexture::Initialize(ID3D11Device* pID3D11Device, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	//initialize render target description
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	//setup
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //this makes this texture render target
	textureDesc.CPUAccessFlags = NULL;
	textureDesc.MiscFlags = NULL;

	//create render target texture
	hr = pID3D11Device->CreateTexture2D(&textureDesc, NULL, &mpRenderTargetTexture);

	if (FAILED(hr))
	{
		return hr;
	}

	//render target view description
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	//create render target view
	hr = pID3D11Device->CreateRenderTargetView(mpRenderTargetTexture, &renderTargetViewDesc, &mpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		return hr;
	}

	//shader resource view
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;


	//create shader resource view
	hr = pID3D11Device->CreateShaderResourceView(mpRenderTargetTexture, &shaderResourceViewDesc, &mpID3D11ShaderResourceView);

	if (FAILED(hr))
	{
		return hr;
	}
}


void RenderToTexture::Uninitialize()
{
	if (mpRenderTargetTexture)
	{
		mpRenderTargetTexture->Release();
		mpRenderTargetTexture = NULL;
	}
	

	if (mpID3D11RenderTargetView)
	{
		mpID3D11RenderTargetView->Release();
		mpID3D11RenderTargetView = NULL;
	}

	if (mpID3D11ShaderResourceView)
	{
		mpID3D11ShaderResourceView->Release();
		mpID3D11ShaderResourceView = NULL;
	}
}



void RenderToTexture::SetRenderTargetView(ID3D11DeviceContext* pID3D11DeviceContext, ID3D11DepthStencilView* pID3D11DepthStecilView)
{
	pID3D11DeviceContext->OMSetRenderTargets(1, &mpID3D11RenderTargetView, pID3D11DepthStecilView);

}


void RenderToTexture::ClearRenderTargetView(ID3D11DeviceContext* pID3D11DeviceContext, ID3D11DepthStencilView* pID3D11DepthStecilView , float r , float g , float b , float a)
{
	float color[4];
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;

	//clear back buffer
	pID3D11DeviceContext->ClearRenderTargetView(mpID3D11RenderTargetView, color);

	//clear depth buffer
	pID3D11DeviceContext->ClearDepthStencilView(pID3D11DepthStecilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


}


ID3D11ShaderResourceView* RenderToTexture::GetShaderResourceView()
{
	return mpID3D11ShaderResourceView;
}