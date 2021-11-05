#include "common.h"
#include "Atmosphere.h"
#include "renderer.h"




namespace renderer
{
	ID3D11BlendState* pID3D11BlendState01 = nullptr;
	ID3D11BlendState* pID3D11BlendState0011 = nullptr;
	ID3D11SamplerState* pID3D11SamplerState = nullptr;
	ID3D11RasterizerState* pID3D11RasterizerState = nullptr;
	ID3D11DepthStencilState* pID3D11DepthStencilState = nullptr;
	ID3D11RenderTargetView* pID3D11RenderTargetView = nullptr;
	ID3D11Buffer* pConstantBuffer= nullptr;

	//Earth texture here
	ID3D11ShaderResourceView* earth_texture;
	

	static unsigned int width;
	static unsigned int height;
	static float fov_y_angle_deg = 50.f;
	static float near_plane = 1.0;

	struct cbuffer
	{
		DirectX::XMFLOAT4X4 view_from_clip;
		DirectX::XMFLOAT4X4 world_from_view;
		DirectX::XMFLOAT3 view_pos_ws;
		float sun_disk_size_x;
		DirectX::XMFLOAT3 earth_center_pos_ws;
		float sun_disk_size_y;
		DirectX::XMFLOAT3 sun_direction_ws;
		float exposure;
		DirectX::XMFLOAT3 white_point;
		int layer;
		DirectX::XMFLOAT4X4 luminance_from_radiance;
		int scaterring_order;
		float _pad[3];
	};


	

	cbuffer atmosphere_cb;


	HRESULT create_texture_2d(ID3D11Device* pID3D11Device, unsigned int width, unsigned int height, D3D11_SUBRESOURCE_DATA* p_init_data, DXGI_FORMAT format, Texture2D* p_out_texture)
	{


		if (p_out_texture->mpTexture2D)
		{
			p_out_texture->mpRenderTargetView->Release();
			p_out_texture->mpTexture2D->Release();
			p_out_texture->mpTextureResourceView->Release();

			p_out_texture->mpRenderTargetView = 0;
			p_out_texture->mpTexture2D = 0;
			p_out_texture->mpTextureResourceView = 0;

		}
		
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
			MessageBox(NULL, L"renderer::CreateTexture2D failed", L"msg", MB_OK);
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
		if (p_out_texture->mpTexture3D)
		{
			p_out_texture->mpRenderTargetView->Release();
			p_out_texture->mpTexture3D->Release();
			p_out_texture->mpTextureResourceView->Release();

			p_out_texture->mpRenderTargetView = 0;
			p_out_texture->mpTexture3D = 0;
			p_out_texture->mpTextureResourceView = 0;

		}
		
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


	HRESULT create_cb(ID3D11Device* pID3D11Device, ID3D11Buffer** ppID3D11Buffer, const void* p_data, unsigned int size)
	{
		/*
		D3D11_BUFFER_DESC cb_desc;
		ZeroMemory(&cb_desc, sizeof(D3D11_BUFFER_DESC));

		cb_desc.ByteWidth = size;
		cb_desc.Usage = D3D11_USAGE_DYNAMIC;
		cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = pID3D11Device->CreateBuffer(&cb_desc, nullptr, ppID3D11Buffer);

		if (FAILED(hr))
		{
			MessageBox(NULL, L"problem in creating buffer", L"error", MB_OK);
			return hr;
		}
		*/

		D3D11_BUFFER_DESC cb_desc = {};
		cb_desc.ByteWidth = size;
		cb_desc.Usage = D3D11_USAGE_DYNAMIC;
		cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA cb_init_data;
		ZeroMemory(&cb_init_data, sizeof(D3D11_SUBRESOURCE_DATA));
		cb_init_data.pSysMem = p_data;
		
		HRESULT h_result = pID3D11Device->CreateBuffer(&cb_desc, NULL, ppID3D11Buffer);
		
		if (FAILED(h_result))
		{
			MessageBox(NULL, L"failed to create the buffer", L"error", MB_OK);
			uninitialize();
			return h_result;
		}
		return S_OK; 
	}


	HRESULT update_cb(ID3D11DeviceContext* pID3D11DeviceContext, ID3D11Buffer** ppID3D11Buffer, const void* p_data, unsigned int data_size)
	{
		

		/*
		D3D11_MAPPED_SUBRESOURCE mapped_subresource;

		ZeroMemory(&mapped_subresource,sizeof(D3D11_MAPPED_SUBRESOURCE));

		if (*ppID3D11Buffer != NULL)
		{
		
			HRESULT hr = pID3D11DeviceContext->Map(*ppID3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
		}
		else
		{
			MessageBox(NULL, L"you are sending a null buffer", L"eror", MB_OK);

		}
		
		memcpy(mapped_subresource.pData, p_data, data_size);

		pID3D11DeviceContext->Unmap(*ppID3D11Buffer, 0);

		//MessageBox(NULL, L"renderer update done success", L"eror", MB_OK);
		*/

		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		ZeroMemory(&mapped_resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		HRESULT h_result = pID3D11DeviceContext->Map(*ppID3D11Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
		//if (h_result != S_OK) error_win32("Map", (DWORD)h_result);
		memcpy(mapped_resource.pData, p_data, data_size);
		pID3D11DeviceContext->Unmap(*ppID3D11Buffer, 0);


		return S_OK;
	}


	ID3D11SamplerState* get_sampler_state()
	{
		return pID3D11SamplerState;
	}

	HRESULT init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{


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

		hr = pID3D11Device->CreateRasterizerState(&rasterizer_desc,&pID3D11RasterizerState);
		
		if (FAILED(hr))
		{
			return hr;
		}

		
		ZeroMemory(&atmosphere_cb, sizeof(cbuffer));
		//create the constant buffer
		create_cb(pID3D11Device, &pConstantBuffer, &atmosphere_cb, sizeof(cbuffer));
		 
		D3D11_DEPTH_STENCIL_DESC depthStencil_desc = {};
		hr = pID3D11Device->CreateDepthStencilState(&depthStencil_desc, &pID3D11DepthStencilState);
		if (hr != S_OK)
		{
			return hr;
		}

		//Load the earth texture
		LoadD3DTexture(pID3D11Device, pID3D11DeviceContext, L"TextureLoaders/earth_nasa_no_cloud.jpg", &earth_texture);

		return S_OK;
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

	ID3D11DepthStencilState* get_depth_stencil_state()
	{
		return pID3D11DepthStencilState;
	}


	void resize(unsigned int width , unsigned int height )
	{
		renderer::width = width;
		renderer::height = height;		

		
	}


	void update(ID3D11Device* pID3D11Device,  ID3D11DeviceContext* pID3D11DeviceContext , float zenithAngle , float azimuthAngle ,unsigned int key)
	{

	

		float fov_y_angle_rad = XMConvertToRadians(fov_y_angle_deg);
		float aspect_ratio = (float)width / (float)height;
		float scale_y = (float)(1.0 / tan(fov_y_angle_rad / 2.0));
		float scale_x = scale_y / aspect_ratio;

		
		float	view_distance = 9000.0f;
		float	view_zenith_angle_in_degrees = XMConvertToDegrees(1.47f);
		float	view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
		float	sun_zenith_angle_in_degrees = XMConvertToDegrees(1.3f + zenithAngle);
		float	sun_azimuth_angle_in_degrees = XMConvertToDegrees(3.0f + azimuthAngle);
		float	exposure = 10.0f;

		if (key == 0)
		{
			view_distance = 9000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.47f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.564f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(-3.0f + azimuthAngle);
			exposure = 10.0f;
		}
	
		else if (key == 1)
		{
			view_distance = 7000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.57f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.54f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(-2.96f + azimuthAngle);
			exposure = 10.0f;
		}
		else if (key == 2)
		{
			view_distance = 7000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.57f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.54f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(-3.044f + azimuthAngle);
			exposure = 10.0f;

		}
		else if (key == 3)
		{
			view_distance = 7000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.57f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.328f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(-2.96f + azimuthAngle);
			exposure = 10.0f;
		}
		else if (key == 4)
		{
			
			view_distance = 7000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.43f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.57f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(1.34f + azimuthAngle);
			exposure = 40.0f;
		}
		else if (key == 5)
		{
		

			view_distance = 9000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.5f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.628f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(1.05f + azimuthAngle);
			exposure = 200.0f;
		}
		else if (key == 6)
		{

			view_distance = 9000.0f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(1.39f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.2f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(0.7f + azimuthAngle);
			exposure = 10.0f;

		}
		else if (key == 7)
		{
			
			view_distance = 1.2e7f;
			view_zenith_angle_in_degrees = XMConvertToDegrees(0.0f);
			view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
			sun_zenith_angle_in_degrees = XMConvertToDegrees(1.93f + zenithAngle);
			sun_azimuth_angle_in_degrees = XMConvertToDegrees(-2.0f + azimuthAngle);
			exposure = 10.0f;
		}
		else if(key == 8)
		{
			
				view_distance = 2.7e6f;
				view_zenith_angle_in_degrees = XMConvertToDegrees(0.81f);
				view_azimuth_angle_in_degrees = XMConvertToDegrees(0.0f);
				sun_zenith_angle_in_degrees = XMConvertToDegrees(1.57f + zenithAngle);
				sun_azimuth_angle_in_degrees = XMConvertToDegrees(2.0f + azimuthAngle);
				exposure = 10.0f;
		}


		double white_point_r = 1.0;
		double white_point_g = 1.0;
		double white_point_b = 1.0;

		const double sun_angular_radius_rad = 0.00935 / 2.0;

		
		DirectX::XMFLOAT4X4 clip_from_view = { // left-handed reversed-z infinite projection
						scale_x, 0.0, 0.0, 0.0,
						0.0, scale_y, 0.0, 0.0,
						0.0, 0.0, 0.0, near_plane,
						0.0, 0.0, 1.0, 0.0
		};

		XMMATRIX view_from_clip = XMMatrixInverse(nullptr, XMLoadFloat4x4(&clip_from_view));
		
		DirectX::XMStoreFloat4x4(&(atmosphere_cb.view_from_clip), view_from_clip);

		
		float cos_theta = (float)cos(XMConvertToRadians(view_zenith_angle_in_degrees));
		float sin_theta = (float)sin(XMConvertToRadians(view_zenith_angle_in_degrees));
		float cos_phi = (float)cos(XMConvertToRadians(view_azimuth_angle_in_degrees));
		float sin_phi = (float)sin(XMConvertToRadians(view_azimuth_angle_in_degrees));

		DirectX::XMFLOAT3 view_x_ws = { -sin_phi, cos_phi, 0.0f };
		DirectX::XMFLOAT3 view_y_ws = { -cos_theta * cos_phi, -cos_theta * sin_phi, sin_theta };
		DirectX::XMFLOAT3 view_z_ws = { -sin_theta * cos_phi, -sin_theta * sin_phi, -cos_theta };
	
		DirectX::XMFLOAT4X4 world_from_view = {
		view_x_ws.x, view_y_ws.x, view_z_ws.x, -view_z_ws.x * view_distance / 1000.f,
		view_x_ws.y, view_y_ws.y, view_z_ws.y, -view_z_ws.y * view_distance / 1000.f,
		view_x_ws.z, view_y_ws.z, view_z_ws.z, -view_z_ws.z * view_distance / 1000.f,
		0.0, 0.0, 0.0, 1.0
		};

		atmosphere_cb.world_from_view = world_from_view;
		atmosphere_cb.view_pos_ws = { world_from_view(0,3),world_from_view(1,3),world_from_view(2,3) };
		atmosphere_cb.earth_center_pos_ws = { 0.0f,0.0f,-6360.0 };

		cos_theta = (float)cos(XMConvertToRadians(sun_zenith_angle_in_degrees));
		sin_theta = (float)sin(XMConvertToRadians(sun_zenith_angle_in_degrees));
		cos_phi =   (float)cos(XMConvertToRadians(sun_azimuth_angle_in_degrees));
		sin_phi =   (float)sin(XMConvertToRadians(sun_azimuth_angle_in_degrees));
		
		atmosphere_cb.sun_direction_ws =
		{
			cos_phi * sin_theta,
			sin_phi * sin_theta,
			cos_theta
		};


		//white balance ?
		atmosphere_cb.white_point = { (float)white_point_r, (float)white_point_g , (float)white_point_b };
		
		
		atmosphere_cb.sun_disk_size_x = (float)tan(sun_angular_radius_rad);
		atmosphere_cb.sun_disk_size_y = (float)cos(sun_angular_radius_rad);
		atmosphere_cb.exposure = exposure;
		
		
	//	pID3D11DeviceContext->UpdateSubresource(pConstantBuffer,0,0,&atmosphere_cb,0,0);

		update_cb(pID3D11DeviceContext,&pConstantBuffer, &atmosphere_cb, sizeof(atmosphere_cb));
	}



	void draw(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{
		//before this
		//clear the color
		//clear render target view

		pID3D11DeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		pID3D11DeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
		//update(pID3D11Device, pID3D11DeviceContext);

		pID3D11DeviceContext->VSSetShader(atmosphere::get_atmosphere_demo_vs(),0,0);

		
		D3D11_VIEWPORT viewport = { 0.0,0.0,(float)width, (float)height,0.0,1.0 };
		pID3D11DeviceContext->RSSetViewports(1, &viewport);
		pID3D11DeviceContext->RSSetState(get_rasterizer_state());
		ID3D11SamplerState *sampler = get_sampler_state();
		pID3D11DeviceContext->PSSetSamplers(0, 1, &sampler);

		
		
		ID3D11ShaderResourceView* a_srv[] =
		{
			atmosphere::get_transmittance_texture().mpTextureResourceView,
			atmosphere::get_scattering_texture().mpTextureResourceView,
			atmosphere::get_single_mie_scattering_texture().mpTextureResourceView,
			atmosphere::get_irradiance_texture().mpTextureResourceView,
			earth_texture
			
		};
	
		pID3D11DeviceContext->PSSetShaderResources(0, 5, a_srv);
		
		pID3D11DeviceContext->OMSetDepthStencilState(get_depth_stencil_state(), 0);

		pID3D11DeviceContext->PSSetShader(atmosphere::get_atmosphere_demo_ps(),0,0);
		

		pID3D11DeviceContext->Draw(4, 0);

		ID3D11ShaderResourceView* const a_null_srv[4] = {};
		pID3D11DeviceContext->PSSetShaderResources(0, 4, a_null_srv);
		

		//after this present frame
	}


	HRESULT LoadD3DTexture(ID3D11Device* pID3D11Device , ID3D11DeviceContext* pID3D11DeviceContext,const wchar_t* textureFileName, ID3D11ShaderResourceView** ppID3D11ShaderResourceView)
	{
		HRESULT hr;

		//creating texture directly from the source texture file itself
		hr = DirectX::CreateWICTextureFromFile(pID3D11Device, pID3D11DeviceContext, textureFileName, NULL, ppID3D11ShaderResourceView);

		if (FAILED(hr))
		{
			return hr;
		}


		return S_OK;
	}


	void uninitialize()
	{
		if (pConstantBuffer)
		{
			pConstantBuffer->Release();
			pConstantBuffer = NULL;
		}

		if (pID3D11BlendState01)
		{
			pID3D11BlendState01->Release();
			pID3D11BlendState01 = NULL;

		}

		if (pID3D11BlendState0011)
		{
			pID3D11BlendState0011->Release();
			pID3D11BlendState0011 = NULL;

		}


		if (pID3D11SamplerState)
		{
			pID3D11SamplerState->Release();
			pID3D11SamplerState = NULL;

		}

		if (pID3D11RasterizerState)
		{
			pID3D11RasterizerState->Release();
			pID3D11RasterizerState = NULL;

		}

		if (pID3D11DepthStencilState)
		{
			pID3D11DepthStencilState->Release();
			pID3D11DepthStencilState = NULL;

		}

		if (pID3D11RenderTargetView)
		{
			pID3D11RenderTargetView->Release();
			pID3D11RenderTargetView = NULL;

		}
	
	
	
	}
}





