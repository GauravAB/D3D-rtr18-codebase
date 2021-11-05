#pragma once
#include "common.h"

namespace renderer
{
	struct Texture2D
	{
		ID3D11Texture2D* mpTexture2D;
		ID3D11ShaderResourceView* mpTextureResourceView;
		ID3D11RenderTargetView* mpRenderTargetView;

		ID3D11Texture2D** getAddressOfTexture()
		{
			return &mpTexture2D;
		}

		ID3D11ShaderResourceView** getAddressOfSRV()
		{
			return &mpTextureResourceView;
		}

		ID3D11RenderTargetView** getAddressOfRTV()
		{
			return &mpRenderTargetView;
		}

	};

	struct Texture3D
	{
		ID3D11Texture3D* mpTexture3D;
		ID3D11ShaderResourceView* mpTextureResourceView;
		ID3D11RenderTargetView* mpRenderTargetView;

		ID3D11Texture3D** getAddressOfTexture()
		{
			return &mpTexture3D;
		}

		ID3D11ShaderResourceView** getAddressOfSRV()
		{
			return &mpTextureResourceView;
		}

		ID3D11RenderTargetView** getAddressOfRTV()
		{
			return &mpRenderTargetView;
		}
	};

	ID3D11BlendState* get_blend_state01();
	ID3D11BlendState* get_blend_state0011();
	ID3D11SamplerState* get_sampler_state();
	ID3D11RasterizerState* get_rasterizer_state();
	ID3D11DepthStencilState* get_depth_stencil_state();


	HRESULT init(ID3D11Device*, ID3D11DeviceContext*);

	HRESULT create_texture_2d(ID3D11Device* ,unsigned int width, unsigned int height, D3D11_SUBRESOURCE_DATA* p_init_data, DXGI_FORMAT format, Texture2D* p_out_texture);
	HRESULT create_texture_3d(ID3D11Device* ,unsigned int width, unsigned int height, unsigned int depth, D3D11_SUBRESOURCE_DATA* p_init_data, DXGI_FORMAT format, Texture3D* p_out_texture);

	HRESULT create_cb(ID3D11Device* ,ID3D11Buffer**, const void* p_data, unsigned int size);
	HRESULT update_cb(ID3D11DeviceContext* ,ID3D11Buffer**, const void* p_data, unsigned int data_size);

	void draw(ID3D11Device*, ID3D11DeviceContext*);

	void resize(unsigned int width, unsigned int height);

	void update(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , float zenithAngle , float azimuthAngle , unsigned int key);

	HRESULT LoadD3DTexture(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, const wchar_t* textureFileName, ID3D11ShaderResourceView** ppID3D11ShaderResourceView);

	void uninitialize();

}

