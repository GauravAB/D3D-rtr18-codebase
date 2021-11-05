#include "common.h"
#include "Atmosphere.h"
#include "renderer.h"



namespace renderer
{
	ID3D11BlendState* pID3D11BlendState01 = nullptr;
	ID3D11BlendState* pID3D11BlendState0011 = nullptr;
	ID3D11SamplerState* pID3D11SamplerState = nullptr;
	ID3D11RasterizerState* pID3D11RasterizerState = nullptr;
	static unsigned int width;
	static unsigned int height;
	static float fov_y_angle_deg = 50.f;
	static float near_plane = 1.0;

	struct AtmosphereConstantBuffer
	{
		struct
		{
			XMFLOAT4X4 view_from_clip;
			XMFLOAT4X4 world_from_view;
			XMFLOAT3 view_pos_ws;
			float sun_disk_size_x;
			XMFLOAT3 earth_center_pos_ws;
			float sun_disk_size_y;
			XMFLOAT3 sun_direction_ws;
			float exposure;
			XMFLOAT3 white_point;
			int layer;
			XMFLOAT4X4 luminance_from_radiance;
			int scaterring_order;
			float _pad[3];
		}data;
		ID3D11Buffer* pConstantBuffer;
	} atmosphere_cb = {};


	HRESULT create_texture_2d(ID3D11Device* pID3D11Device, unsigned int width, unsigned int height, D3D11_SUBRESOURCE_DATA* p_init_data, DXGI_FORMAT format, Texture2D* p_out_texture)
	{
		p_out_texture->mpRenderTargetView->Release();
		p_out_texture->mpTexture2D->Release();
		p_out_texture->mpTextureResourceView->Release();

		p_out_texture->mpRenderTargetView = 0;
		p_out_texture->mpTexture2D = 0;
		p_out_texture->mpTextureResourceView = 0;

		HRESULT hr = 0;


		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //as a render target


		hr = pID3D11Device->CreateTexture2D(&desc, p_init_data, p_out_texture->getAddressOfTexture());

		if (FAILED(hr))
		{
			return hr;
		}


		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;

		if (p_out_texture->mpTexture2D)
		{
			hr = pID3D11Device->CreateShaderResourceView(p_out_texture->mpTexture2D, &srv_desc, p_out_texture->getAddressOfSRV());
		}

		if (FAILED(hr))
		{
			return hr;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		if (p_out_texture->mpTexture2D)
		{
			hr = pID3D11Device->CreateRenderTargetView(p_out_texture->mpTexture2D, &rtv_desc, p_out_texture->getAddressOfRTV());
		}

		if (FAILED(hr))
		{
			return hr;
		}

		return NULL;
	}

	HRESULT create_texture_3d(ID3D11Device* pID3D11Device, unsigned int width, unsigned int height, unsigned int depth, D3D11_SUBRESOURCE_DATA* p_init_data, DXGI_FORMAT format, Texture3D* p_out_texture)
	{
		p_out_texture->mpRenderTargetView->Release();
		p_out_texture->mpTexture3D->Release();
		p_out_texture->mpTextureResourceView->Release();

		p_out_texture->mpRenderTargetView = 0;
		p_out_texture->mpTexture3D = 0;
		p_out_texture->mpTextureResourceView = 0;

		HRESULT hr = 0;
		D3D11_TEXTURE3D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.Depth = depth;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		hr = pID3D11Device->CreateTexture3D(&desc, p_init_data, p_out_texture->getAddressOfTexture());

		if (FAILED(hr))
		{
			return hr;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		srv_desc.Texture3D.MipLevels = 1;

		if (p_out_texture->mpTexture3D)
		{
			hr = pID3D11Device->CreateShaderResourceView(p_out_texture->mpTexture3D, &srv_desc, p_out_texture->getAddressOfSRV());
		}

		if (FAILED(hr))
		{
			return hr;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		rtv_desc.Texture3D.WSize = -1;

		if (p_out_texture->mpTexture3D)
		{
			hr = pID3D11Device->CreateRenderTargetView(p_out_texture->mpTexture3D, &rtv_desc, p_out_texture->getAddressOfRTV());
		}

		if (FAILED(hr))
		{
			return hr;
		}

		return NULL;

	}


	HRESULT create_cb(ID3D11Device* pID3D11Device, ID3D11Buffer* pID3D11Buffer, const void* p_data, unsigned int size)
	{
		D3D11_BUFFER_DESC cb_desc = {};
		cb_desc.ByteWidth = size;
		cb_desc.Usage = D3D11_USAGE_DYNAMIC;
		cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA cb_init_data = {};
		cb_init_data.pSysMem = p_data;

		HRESULT hr = pID3D11Device->CreateBuffer(&cb_desc, &cb_init_data, &pID3D11Buffer);

		if (FAILED(hr))
		{
			return hr;
		}
	}


	HRESULT update_cb(ID3D11DeviceContext* pID3D11DeviceContext, ID3D11Buffer* pID3D11Buffer, const void* p_data, unsigned int data_size)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_subresource;
		ZeroMemory(&mapped_subresource, sizeof(mapped_subresource));
;		HRESULT hr = pID3D11DeviceContext->Map(pID3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
		if (FAILED(hr))
		{
			return hr;
		}

		memcpy(mapped_subresource.pData, p_data, data_size);

		pID3D11DeviceContext->Unmap(pID3D11Buffer, 0);
	}


	ID3D11SamplerState* get_sampler_state()
	{
		return pID3D11SamplerState;
	}

	HRESULT init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{
		quad::init(pID3D11Device,pID3D11DeviceContext);


		HRESULT hr;

		{
			D3D11_BLEND_DESC blend_state_desc = {};
			blend_state_desc.IndependentBlendEnable = true;
			blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			blend_state_desc.RenderTarget[1].BlendEnable = true;
			blend_state_desc.RenderTarget[1].SrcBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[1].DestBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			hr= pID3D11Device->CreateBlendState(&blend_state_desc, &pID3D11BlendState01);
			
			if (FAILED(hr))
			{
				return hr;
			}
		}


		{
			D3D11_BLEND_DESC blend_state_desc = {};
			blend_state_desc.IndependentBlendEnable = true;
			blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			blend_state_desc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			blend_state_desc.RenderTarget[2].BlendEnable = true;
			blend_state_desc.RenderTarget[2].SrcBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[2].DestBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[2].BlendOp = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[2].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[2].DestBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[2].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			blend_state_desc.RenderTarget[3].BlendEnable = true;
			blend_state_desc.RenderTarget[3].SrcBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[3].DestBlend = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[3].BlendOp = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[3].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[3].DestBlendAlpha = D3D11_BLEND_ONE;
			blend_state_desc.RenderTarget[3].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_state_desc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			hr = pID3D11Device->CreateBlendState(&blend_state_desc, &pID3D11BlendState0011);
			
			if (FAILED(hr))
			{
				return hr;
			}
		}


		D3D11_SAMPLER_DESC sampler_desc;
		ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));

		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;


		hr = pID3D11Device->CreateSamplerState(&sampler_desc, &pID3D11SamplerState);
		if (FAILED(hr))
		{
			return hr;
		}


		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = true;

		HRESULT hr = pID3D11Device->CreateRasterizerState(&rasterizer_desc,&pID3D11RasterizerState);
		
		if (FAILED(hr))
		{
			return hr;
		}


		create_cb(pID3D11Device, atmosphere_cb.pConstantBuffer,&atmosphere_cb.data, sizeof(atmosphere_cb.data));
	}


	ID3D11BlendState* get_blend_state01()
	{
		return pID3D11BlendState01;
	}

	ID3D11BlendState* get_blend_state0011()
	{
		return pID3D11BlendState0011;
	}
	
	ID3D11RasterizerState* get_rasterizer_state()
	{
		return pID3D11RasterizerState;
	}


	void resize(unsigned int width , unsigned int height)
	{
		renderer::width = width;
		renderer::height = height;

	}


	void update(ID3D11Device* pID3D11Device,  ID3D11DeviceContext* pID3D11DeviceContext)
	{
		float fov_y_angle_rad = XMConvertToRadians(fov_y_angle_deg);
		float aspect_ratio = (float)width / (float)height;
		float scale_y = (float)(1.0 / tan(fov_y_angle_rad / 2.0));
		float scale_x = scale_y / aspect_ratio;

		XMFLOAT4X4 clip_from_view = {
			scale_x, 0.0,	  0.0, 0.0,
			0.0,	 scale_y, 0.0, 0.0,
			0.0,     0.0,     0.0, near_plane,
			0.0,     0.0,     1.0, 0.0
		};

		XMMATRIX view_from_clip = XMMatrixInverse(nullptr, XMLoadFloat4x4(&clip_from_view));
		XMStoreFloat4x4(&atmosphere_cb.data.view_from_clip, view_from_clip);


		float view_zenith_angle_in_degrees = 0.81f;
		float view_azimuth_angle_in_degree = 0.0f;
		float view_distance = 2.7e6f;

		float cos_theta = (float)cos(XMConvertToRadians(view_zenith_angle_in_degrees));
		float sin_theta = (float)sin(XMConvertToRadians(view_zenith_angle_in_degrees));
		float cos_phi = (float)cos(XMConvertToRadians(view_azimuth_angle_in_degree));
		float sin_phi = (float)sin(XMConvertToRadians(view_azimuth_angle_in_degree));

		XMFLOAT3 view_x_ws = { -sin_phi, cos_phi, 0.f };
		XMFLOAT3 view_y_ws = { -cos_theta * cos_phi, -cos_theta * sin_phi, sin_theta };
		XMFLOAT3 view_z_ws = { -sin_theta * cos_phi, -sin_theta * sin_phi, -cos_theta };

		XMFLOAT4X4 world_from_view = {
		view_x_ws.x, view_y_ws.x, view_z_ws.x, -view_z_ws.x * view_distance / 1000.f,
		view_x_ws.y, view_y_ws.y, view_z_ws.y, -view_z_ws.y * view_distance / 1000.f,
		view_x_ws.z, view_y_ws.z, view_z_ws.z, -view_z_ws.z * view_distance / 1000.f,
		0.0, 0.0, 0.0, 1.0
		};

		atmosphere_cb.data.world_from_view = world_from_view;
		atmosphere_cb.data.earth_center_pos_ws = { 0.0f,0.0f,-6350.0 };


		float sun_zenith_angle_in_degrees = 1.57f;
		float sun_azimuth_angle_in_degrees = 2.0f;

		cos_theta = (float)cos(XMConvertToRadians(sun_zenith_angle_in_degrees));
		sin_theta = (float)sin(XMConvertToRadians(sun_zenith_angle_in_degrees));
		cos_phi =   (float)cos(XMConvertToRadians(sun_azimuth_angle_in_degrees));
		sin_phi =   (float)sin(XMConvertToRadians(sun_azimuth_angle_in_degrees));
		
		atmosphere_cb.data.sun_direction_ws =
		{
			cos_phi * sin_theta,
			sin_phi * sin_theta,
			cos_theta
		};


		//white balance ?
		double white_point_r = 1.0;
		double white_point_g = 1.0;
		double white_point_b = 1.0;

		atmosphere_cb.data.white_point = { (float)white_point_r, (float)white_point_g , (float)white_point_b };
		
		float exposure = 10.0f;
		const double sun_angular_radius_rad = 0.00935 / 2.0;
		atmosphere_cb.data.sun_disk_size_x = (float)tan(sun_angular_radius_rad);
		atmosphere_cb.data.sun_disk_size_y = (float)cos(sun_angular_radius_rad);
		atmosphere_cb.data.exposure = exposure;
		
		update_cb(pID3D11DeviceContext,atmosphere_cb.pConstantBuffer, &atmosphere_cb.data, sizeof(atmosphere_cb.data));
	}



	void draw(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{
		//before this
		//clear the color
		//clear render target view
		
		quad::setVertexBufferAndTopology(pID3D11Device,pID3D11DeviceContext);
		ID3D11Buffer* temp = atmosphere_cb.pConstantBuffer;

		pID3D11DeviceContext->VSSetConstantBuffers(0, 1, &temp);
		pID3D11DeviceContext->VSSetShader(atmosphere::get_atmosphere_demo_vs(),0,0);

		D3D11_VIEWPORT viewport = { 0.0,0.0,(float)width, (float)height,0.0,1.0 };
		pID3D11DeviceContext->RSSetViewports(1, &viewport);
		pID3D11DeviceContext->RSSetState(get_rasterizer_state());
		pID3D11DeviceContext->PSSetConstantBuffers(0, 1, &temp);
		ID3D11SamplerState *sampler = get_sampler_state();
		pID3D11DeviceContext->PSSetSamplers(0, 1, &sampler);
		ID3D11ShaderResourceView* a_srv[] =
		{
			atmosphere::get_transmittance_texture().getAddressOfSRV,
			atmosphere::get_scattering_texture().getAddressOfSRV,
			atmosphere::get_single_mie_scattering_texture().getAddressOfSRV,
			atmosphere::get_irradiance_texture().getAddressOfSRV
		};

		pID3D11DeviceContext->PSSetShaderResources(0, 4, a_srv);
		pID3D11DeviceContext->PSSetShader(atmosphere::get_atmosphere_demo_ps(),0,0);
		
		pID3D11DeviceContext->Draw(4, 0);

		ID3D11ShaderResourceView* const a_null_srv[4] = {};
		pID3D11DeviceContext->PSSetShaderResources(0, 4, a_null_srv);

		//after this present frame
	}


}





