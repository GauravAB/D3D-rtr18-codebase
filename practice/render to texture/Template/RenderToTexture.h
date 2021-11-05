#pragma once
#include "common.h"



class RenderToTexture
{

public:
	RenderToTexture();
	RenderToTexture(const RenderToTexture&);
	~RenderToTexture();

	HRESULT Initialize(ID3D11Device*, int, int);
	void Uninitialize();

	void SetRenderTargetView(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearRenderTargetView(ID3D11DeviceContext*, ID3D11DepthStencilView*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();



private:

	//render to texture stuff
	ID3D11Texture2D* mpRenderTargetTexture;
	ID3D11ShaderResourceView* mpID3D11ShaderResourceView;
	ID3D11RenderTargetView* mpID3D11RenderTargetView;


};