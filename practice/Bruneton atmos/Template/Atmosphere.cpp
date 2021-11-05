#include "Atmosphere.h"
#include "common.h"
#include "renderer.h"
#include <cmath>
#include <assert.h>
#include <cstring>



typedef double scalar;
typedef double3 scalar3;
#include "atmosphere_definitions.h"

namespace atmosphere
{

	ID3D11VertexShader* atmosphere_demo_vs;
	ID3D11PixelShader* atmosphere_demo_ps;

	ID3D11VertexShader* precompute_vertexshader;
	ID3D11InputLayout* precompute_vertexshader_input_layout;

	//precomputation in pixel shader
	ID3D11PixelShader* precompute_pixelshader_transmittance = nullptr;
	ID3D11PixelShader* precompute_pixelshader_delta_irradiance = nullptr;
	ID3D11PixelShader* precompute_pixelshader_single_scattering = nullptr;
	ID3D11PixelShader* precompute_pixelshader_scattering_density = nullptr;
	ID3D11PixelShader* precompute_pixelshader_indirect_irradiance = nullptr;
	ID3D11PixelShader* precompute_pixelshader_multiple_scaterring = nullptr;
	ID3D11Buffer* pPrecomputeConstantBuffer = nullptr;

	//herecomes the geometry shader !
	ID3D11GeometryShader* precompute_geometry_shader = nullptr;

	renderer::Texture2D transmittance_texture;
	renderer::Texture2D irradiance_texture;
	renderer::Texture2D delta_irradiance_texture;
	renderer::Texture3D scattering_texture;
	renderer::Texture3D single_mie_scattering_texture;
	renderer::Texture3D delta_rayleigh_scattering_texture;
	renderer::Texture3D delta_mie_scattering_texture;
	renderer::Texture3D delta_scattering_density_texture;

	renderer::Texture2D test_irradiance_texture;
	renderer::Texture2D test_delta_irradiance_texture;

	
	struct cbuffer
	{
		XMFLOAT4X4 luminance_from_radiance;
		int scattering_order;
		int test;
		float _pad[2];
	};

	//constant buffer for precompute shader
	cbuffer precompute_atmosphere_buffer;

	AtmosphereParameters atmosphere_parameters;
	
	//double sun_k_r, sun_k_g, sun_k_b;
	//double sky_k_r, sky_k_g, sun_k_b;


	ID3D11VertexShader* get_atmosphere_demo_vs()
	{
		return atmosphere_demo_vs;
	}

	ID3D11PixelShader* get_atmosphere_demo_ps()
	{
		return atmosphere_demo_ps;
	}


	renderer::Texture2D get_transmittance_texture()
	{
		return transmittance_texture;
	}


	renderer::Texture3D get_scattering_texture()
	{
		return scattering_texture;
	}

	renderer::Texture3D get_single_mie_scattering_texture()
	{
		return single_mie_scattering_texture;
	}

	renderer::Texture2D get_irradiance_texture()
	{
		return irradiance_texture;
	}



	void create_textures(ID3D11Device* pID3D11Device)
	{
		//using fulll precision for now
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		renderer::create_texture_2d(pID3D11Device,TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, nullptr, format, &transmittance_texture);
		
		renderer::create_texture_2d(pID3D11Device,IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, nullptr, format, &irradiance_texture);
		renderer::create_texture_2d(pID3D11Device,IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, nullptr, format, &delta_irradiance_texture);
		renderer::create_texture_3d(pID3D11Device,SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr, format, &scattering_texture);
		renderer::create_texture_3d(pID3D11Device,SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr, format, &single_mie_scattering_texture);
		renderer::create_texture_3d(pID3D11Device,SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr, format, &delta_rayleigh_scattering_texture);
		renderer::create_texture_3d(pID3D11Device,SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr, format, &delta_mie_scattering_texture);
		renderer::create_texture_3d(pID3D11Device,SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, nullptr, format, &delta_scattering_density_texture);
		

	}

	void create_precompute_vertex_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_vertexshader)
		{
			precompute_vertexshader->Release();
			precompute_vertexshader = NULL;
		}
		
		compile_and_create_vertex_shader(pID3D11Device, pID3D11DeviceContext, &precompute_vertexshader,&precompute_vertexshader_input_layout, L"Shaders/vertexShaderPrecompute.hlsl", pFile);
	}



	void create_precompute_pixelshader_transmittance(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , FILE* pFile)
	{
		if (precompute_pixelshader_transmittance)
		{
			precompute_pixelshader_transmittance->Release();
			precompute_pixelshader_transmittance = NULL;

		}

	
		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_transmittance, L"Shaders/pixelShaderTransmittance.hlsl", pFile);
	}

	void create_precompute_pixelshader_delta_irradiance(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_pixelshader_delta_irradiance)
		{
			precompute_pixelshader_delta_irradiance->Release();
			precompute_pixelshader_delta_irradiance = NULL;
		}

		

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext,&precompute_pixelshader_delta_irradiance, L"Shaders/pixelShaderDeltaIrradiance.hlsl", pFile);
	}

	void create_precompute_pixelshader_single_scattering(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_pixelshader_single_scattering)
		{
			precompute_pixelshader_single_scattering->Release();
			precompute_pixelshader_single_scattering = NULL;
		}

		

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_single_scattering, L"Shaders/pixelShaderSingleScattering.hlsl", pFile);
	}

	

	void create_precompute_pixelshader_scattering_density(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_pixelshader_scattering_density)
		{
			precompute_pixelshader_scattering_density->Release();
			precompute_pixelshader_scattering_density = NULL;

		}

	
		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_scattering_density, L"Shaders/pixelShaderScatteringDensity.hlsl", pFile);
	}


	void create_precompute_pixelshader_indirect_irradiance(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_pixelshader_indirect_irradiance)
		{
			precompute_pixelshader_indirect_irradiance->Release();
			precompute_pixelshader_indirect_irradiance = NULL;

		}


		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_indirect_irradiance, L"Shaders/pixelShaderIndirectIrradiance.hlsl", pFile);
	}

	void create_precompute_pixelshader_multiple_scaterring(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_pixelshader_multiple_scaterring)
		{

			precompute_pixelshader_multiple_scaterring->Release();
			precompute_pixelshader_multiple_scaterring = NULL;

		}

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_multiple_scaterring, L"Shaders/pixelShaderMultipleScattering.hlsl", pFile);
	}

	void create_precompute_geometry_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		if (precompute_geometry_shader)
		{

			precompute_geometry_shader->Release();
			precompute_geometry_shader = NULL;

		}

		compile_and_create_geometry_shader(pID3D11Device, pID3D11DeviceContext, &precompute_geometry_shader, L"Shaders/geometryShaderPrecompute.hlsl", pFile);

	}

	void create_demo_shaders(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , FILE* pFile)
	{
		ID3D11InputLayout* pTemp;
		compile_and_create_vertex_shader(pID3D11Device, pID3D11DeviceContext, &atmosphere_demo_vs,&pTemp,L"Shaders/vertexShader.hlsl", pFile);
		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &atmosphere_demo_ps, L"Shaders/pixelShader.hlsl", pFile);
		
		//not using layout
		pTemp->Release();
		pTemp = NULL;

	}

	void precompute_all(ID3D11DeviceContext* pID3D11DeviceContext)
	{

		pID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pID3D11DeviceContext->VSSetShader(precompute_vertexshader, 0, 0);
		ID3D11SamplerState* sampler = renderer::get_sampler_state();
		pID3D11DeviceContext->PSSetSamplers(0, 1, &sampler);
		pID3D11DeviceContext->RSSetState(renderer::get_rasterizer_state());
		pID3D11DeviceContext->OMSetDepthStencilState(renderer::get_depth_stencil_state(), 0);
		ID3D11Buffer* a_cb[] = { pPrecomputeConstantBuffer };
		pID3D11DeviceContext->PSSetConstantBuffers(0, 1, a_cb);


		{
			//Precompute Transmittance

			D3D11_VIEWPORT viewport = { 0.0,0.0,TRANSMITTANCE_TEXTURE_WIDTH,TRANSMITTANCE_TEXTURE_HEIGHT,0.0,1.0 };
			pID3D11DeviceContext->RSSetViewports(1, &viewport);
			pID3D11DeviceContext->PSSetShader(precompute_pixelshader_transmittance, 0, 0);
			ID3D11RenderTargetView* a_rtv[] = { transmittance_texture.mpRenderTargetView };
			pID3D11DeviceContext->OMSetRenderTargets(1, a_rtv, nullptr);
			pID3D11DeviceContext->Draw(4, 0);

			ID3D11RenderTargetView* a_null_rtv[1] = { };
			pID3D11DeviceContext->OMSetRenderTargets(1, a_null_rtv, nullptr);

		}

		
		{
			//Precompute Direct Irradiance
			D3D11_VIEWPORT viewport = { 0.0,0.0,IRRADIANCE_TEXTURE_WIDTH,IRRADIANCE_TEXTURE_HEIGHT,0.0,1.0 };
			pID3D11DeviceContext->RSSetViewports(1, &viewport);
			pID3D11DeviceContext->PSSetShaderResources(0, 1, transmittance_texture.getAddressOfSRV());
			pID3D11DeviceContext->PSSetShader(precompute_pixelshader_delta_irradiance, 0, 0);
			pID3D11DeviceContext->OMSetRenderTargets(1, delta_irradiance_texture.getAddressOfRTV(), nullptr);
			pID3D11DeviceContext->Draw(4, 0);
			ID3D11RenderTargetView* const a_null_rtv = {};
			//detach the rtv
			pID3D11DeviceContext->OMSetRenderTargets(1, &a_null_rtv, nullptr);
			//detach srv
			ID3D11ShaderResourceView* const a_null_srv[1] = { };
			pID3D11DeviceContext->PSSetShaderResources(0, 1, a_null_srv);
		}


		{
			//Precompute Direct Rayleigh and Mie Single Scattering
			D3D11_VIEWPORT viewport = { 0.0,0.0,SCATTERING_TEXTURE_WIDTH,SCATTERING_TEXTURE_HEIGHT,0.0,1.0 };
			pID3D11DeviceContext->RSSetViewports(1, &viewport);
			pID3D11DeviceContext->PSSetShaderResources(0, 1, transmittance_texture.getAddressOfSRV());
			pID3D11DeviceContext->PSSetShader(precompute_pixelshader_single_scattering, 0, 0);
			
			//set shader resource view
			ID3D11ShaderResourceView* a_srv[] =
			{
				transmittance_texture.mpTextureResourceView
			};
			pID3D11DeviceContext->PSSetShaderResources(0, 1, a_srv);
			//added geometry shader
			pID3D11DeviceContext->GSSetShader(precompute_geometry_shader, 0, 0);


			ID3D11RenderTargetView* a_rtv[] = { delta_rayleigh_scattering_texture.mpRenderTargetView,
												delta_mie_scattering_texture.mpRenderTargetView,
												scattering_texture.mpRenderTargetView,
												single_mie_scattering_texture.mpRenderTargetView };
			pID3D11DeviceContext->OMSetRenderTargets(4, a_rtv, nullptr);
			pID3D11DeviceContext->DrawInstanced(4, SCATTERING_TEXTURE_DEPTH, 0, 0);
			ID3D11RenderTargetView* const a_null_rtv[4] = {};
			//detach the rtv
			pID3D11DeviceContext->OMSetRenderTargets(4, a_null_rtv, nullptr);
			//detach the srv
			ID3D11ShaderResourceView* const a_null_srv[1] = { };
			pID3D11DeviceContext->PSSetShaderResources(0, 1, a_null_srv);
		}


		
		
		//Accumulate Multiple Scattering
		{
			for (unsigned int scattering_order = 2; scattering_order <= 4; ++scattering_order)
			{
				
				{
					//Scattering density
					D3D11_VIEWPORT viewport{ 0.0,0.0,SCATTERING_TEXTURE_WIDTH , SCATTERING_TEXTURE_HEIGHT , 0.0 , 1.0 };
					pID3D11DeviceContext->RSSetViewports(1, &viewport);
					ID3D11ShaderResourceView* a_srv[] =
					{
							transmittance_texture.mpTextureResourceView,
							delta_rayleigh_scattering_texture.mpTextureResourceView,
							delta_mie_scattering_texture.mpTextureResourceView,
							delta_rayleigh_scattering_texture.mpTextureResourceView,
							delta_irradiance_texture.mpTextureResourceView
					};
					pID3D11DeviceContext->PSSetShaderResources(0, 5, a_srv);
					pID3D11DeviceContext->PSSetShader(precompute_pixelshader_scattering_density, 0, 0);
					ID3D11RenderTargetView* a_rtv[] = {
						delta_scattering_density_texture.mpRenderTargetView,
					};
					pID3D11DeviceContext->OMSetRenderTargets(1, a_rtv, nullptr);
					//scattering order here

					precompute_atmosphere_buffer.scattering_order = scattering_order;
					renderer::update_cb(pID3D11DeviceContext, &pPrecomputeConstantBuffer, &precompute_atmosphere_buffer,sizeof(cbuffer));


					pID3D11DeviceContext->DrawInstanced(4, SCATTERING_TEXTURE_DEPTH, 0, 0);
					ID3D11RenderTargetView* const a_null_rtv[1] = {};
					//detach the rtv
					pID3D11DeviceContext->OMSetRenderTargets(1, a_null_rtv, nullptr);
					//detach the srv
					ID3D11ShaderResourceView* const a_null_srv[5] = { };
					pID3D11DeviceContext->PSSetShaderResources(0, 5, a_null_srv);
				}
				


				{
					//Indirect Irradiance
					precompute_atmosphere_buffer.scattering_order = scattering_order - 1;
					renderer::update_cb(pID3D11DeviceContext, &pPrecomputeConstantBuffer, &precompute_atmosphere_buffer, sizeof(cbuffer));

					D3D11_VIEWPORT viewport{ 0.0,0.0,IRRADIANCE_TEXTURE_WIDTH , IRRADIANCE_TEXTURE_HEIGHT , 0.0 , 1.0 };
					pID3D11DeviceContext->RSSetViewports(1, &viewport);
					ID3D11ShaderResourceView* a_srvs[] = { delta_rayleigh_scattering_texture.mpTextureResourceView,
														   delta_mie_scattering_texture.mpTextureResourceView,
														   delta_rayleigh_scattering_texture.mpTextureResourceView
					};
					pID3D11DeviceContext->PSSetShaderResources(0, 3, a_srvs);
					pID3D11DeviceContext->PSSetShader(precompute_pixelshader_indirect_irradiance, 0, 0);

					ID3D11RenderTargetView* a_rtv[] = { delta_irradiance_texture.mpRenderTargetView,
														irradiance_texture.mpRenderTargetView,
					};
					pID3D11DeviceContext->OMSetRenderTargets(2, a_rtv, nullptr);
					//---------------------------------------------------------------------------------------------
					//use blend why?
					pID3D11DeviceContext->OMSetBlendState(renderer::get_blend_state01(), NULL, 0xffffffff);
					//------------------------------------------------------------------------------------------------

					pID3D11DeviceContext->Draw(4, 0);

					//clean up
					ID3D11RenderTargetView* const a_null_rtv[2] = {};
					//detach the rtv
					pID3D11DeviceContext->OMSetRenderTargets(2, a_null_rtv, nullptr);
					//detach the srv
					ID3D11ShaderResourceView* const a_null_srv[3] = { };
					pID3D11DeviceContext->PSSetShaderResources(0, 3, a_null_srv);
					pID3D11DeviceContext->OMSetBlendState(nullptr, NULL, 0xffffffff);
				}

				
				{
					//Multiple Scattering
					D3D11_VIEWPORT viewport = { 0.0,0.0,SCATTERING_TEXTURE_WIDTH,SCATTERING_TEXTURE_HEIGHT,0.0,1.0 };
					pID3D11DeviceContext->RSSetViewports(1, &viewport);
					ID3D11ShaderResourceView* a_srv[] = { transmittance_texture.mpTextureResourceView,delta_scattering_density_texture.mpTextureResourceView };
					pID3D11DeviceContext->PSSetShaderResources(0, 2, a_srv);
					pID3D11DeviceContext->PSSetShader(precompute_pixelshader_multiple_scaterring, 0, 0);

					//use blend to when rendering to multiple targets ???
					ID3D11RenderTargetView* a_rtv[] = { delta_rayleigh_scattering_texture.mpRenderTargetView,
														scattering_texture.mpRenderTargetView,
					};
					pID3D11DeviceContext->OMSetRenderTargets(2, a_rtv, nullptr);
					
					//---------------------------------------------------------------------------------------------
					//use blend why?
					pID3D11DeviceContext->OMSetBlendState(renderer::get_blend_state01(), NULL, 0xffffffff);
					//------------------------------------------------------------------------------------------------

					//multiple draws for multiple scatter
					pID3D11DeviceContext->DrawInstanced(4, SCATTERING_TEXTURE_DEPTH, 0, 0);
					ID3D11RenderTargetView* const a_null_rtv[2] = {};
					//detach the rtv
					pID3D11DeviceContext->OMSetRenderTargets(2, a_null_rtv, nullptr);
					ID3D11ShaderResourceView* const a_null_srv[2] = { };
					pID3D11DeviceContext->PSSetShaderResources(0, 2, a_srv);
					pID3D11DeviceContext->OMSetBlendState(nullptr, NULL, 0xffffffff);

				}
				

			}
		}
		
		pID3D11DeviceContext->ClearState();
		pID3D11DeviceContext->Flush();
		
	}
	

	void precompute_transmittance(ID3D11DeviceContext* pID3D11DeviceContext)
	{
		pID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pID3D11DeviceContext->VSSetShader(precompute_vertexshader, 0, 0);
		ID3D11SamplerState* sampler = renderer::get_sampler_state();
		pID3D11DeviceContext->PSSetSamplers(0, 1, &sampler);
		pID3D11DeviceContext->RSSetState(renderer::get_rasterizer_state());
		pID3D11DeviceContext->OMSetDepthStencilState(renderer::get_depth_stencil_state(), 0);
		ID3D11Buffer* a_cb[] = { pPrecomputeConstantBuffer };
		pID3D11DeviceContext->PSSetConstantBuffers(0, 1, a_cb);

		//compute 
		{
			D3D11_VIEWPORT viewport = { 0.0,0.0,TRANSMITTANCE_TEXTURE_WIDTH,TRANSMITTANCE_TEXTURE_HEIGHT,0.0,1.0 };
			pID3D11DeviceContext->RSSetViewports(1, &viewport);
			pID3D11DeviceContext->PSSetShader(precompute_pixelshader_transmittance, 0, 0);
			ID3D11RenderTargetView* a_rtv[] = { transmittance_texture.mpRenderTargetView };
			pID3D11DeviceContext->OMSetRenderTargets(1, a_rtv, nullptr);
			pID3D11DeviceContext->Draw(4, 0);

			ID3D11RenderTargetView* a_null_rtv[1] = { };
			pID3D11DeviceContext->OMSetRenderTargets(1, a_null_rtv, nullptr);
		}

	}

	void create_shaders(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , FILE* pFile)
	{
		MessageBox(NULL, L"atmosphere::inside create shader", L"msg", MB_OK);

		create_precompute_vertex_shader(pID3D11Device,pID3D11DeviceContext,pFile);
		
		create_precompute_geometry_shader(pID3D11Device, pID3D11DeviceContext, pFile);

		create_precompute_pixelshader_transmittance			(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_precompute_pixelshader_delta_irradiance		(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_precompute_pixelshader_single_scattering		(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_precompute_pixelshader_scattering_density	(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_precompute_pixelshader_indirect_irradiance	(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_precompute_pixelshader_multiple_scaterring	(pID3D11Device, pID3D11DeviceContext, pFile);
		
		create_demo_shaders(pID3D11Device, pID3D11DeviceContext,pFile);

	}

		

	void init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , FILE* pFile)
	{
		//create constant buffer

		XMMATRIX identity = XMMatrixIdentity();
		
		
		ZeroMemory(&precompute_atmosphere_buffer, sizeof(atmosphere::cbuffer));

		XMStoreFloat4x4(&(precompute_atmosphere_buffer.luminance_from_radiance), identity);
		
		
		//Create Constant Buffer
		renderer::create_cb(pID3D11Device, &pPrecomputeConstantBuffer, NULL, sizeof(atmosphere::cbuffer));
		//MessageBox(NULL, L"hwnd", L"get lost", MB_OK);
		renderer::update_cb(pID3D11DeviceContext, &pPrecomputeConstantBuffer, &precompute_atmosphere_buffer, sizeof(atmosphere::cbuffer));
	//	MessageBox(NULL, L"hwnd", L"get back", MB_OK);

		create_textures(pID3D11Device);
		
		create_shaders(pID3D11Device, pID3D11DeviceContext,pFile);

		//set precomputed luminance value here
		
		precompute_all(pID3D11DeviceContext);
		precompute_transmittance(pID3D11DeviceContext);
	}
	
	void uninitialize()
	{

		if (pPrecomputeConstantBuffer)
		{
			pPrecomputeConstantBuffer->Release();
			pPrecomputeConstantBuffer = NULL;
		}

		if (atmosphere_demo_vs)
		{
			atmosphere_demo_vs->Release();
			atmosphere_demo_vs = NULL;

		}

		if (atmosphere_demo_ps)
		{
			atmosphere_demo_ps->Release();
			atmosphere_demo_ps = NULL;

		}

		if (precompute_geometry_shader)
		{
			precompute_geometry_shader->Release();
			precompute_geometry_shader = NULL;
		}

		if (precompute_vertexshader)
		{
			precompute_vertexshader->Release();
			precompute_vertexshader = NULL;

		}

		//precomputation in pixel shader safe release
		if (precompute_pixelshader_transmittance)
		{
			precompute_pixelshader_transmittance->Release();
			precompute_pixelshader_transmittance = NULL;

		}
		if (precompute_pixelshader_delta_irradiance) 
		{
			precompute_pixelshader_delta_irradiance->Release();
			precompute_pixelshader_delta_irradiance = NULL;

		}
		if (precompute_pixelshader_single_scattering) 
		{
			precompute_pixelshader_single_scattering->Release();
			precompute_pixelshader_single_scattering = NULL;
		}
		if (precompute_pixelshader_scattering_density) 
		{
			precompute_pixelshader_scattering_density->Release();
			precompute_pixelshader_scattering_density = NULL;
		}
		if (precompute_pixelshader_indirect_irradiance) 
		{
			precompute_pixelshader_indirect_irradiance->Release();
			precompute_pixelshader_indirect_irradiance = NULL;
		}
		if (precompute_pixelshader_multiple_scaterring)
		{
			precompute_pixelshader_multiple_scaterring->Release();
			precompute_pixelshader_multiple_scaterring = NULL;
		}


		if(transmittance_texture.mpTexture2D)
		{
			transmittance_texture.mpTexture2D->Release();
			transmittance_texture.mpTexture2D = NULL;

			transmittance_texture.mpRenderTargetView->Release();
			transmittance_texture.mpRenderTargetView = NULL;

			transmittance_texture.mpTextureResourceView->Release();
			transmittance_texture.mpTextureResourceView = NULL;

		}

		if(irradiance_texture.mpTexture2D)				
		{
			irradiance_texture.mpTexture2D->Release();
			irradiance_texture.mpTexture2D = NULL;

			irradiance_texture.mpRenderTargetView->Release();
			irradiance_texture.mpRenderTargetView = NULL;
			
			irradiance_texture.mpTextureResourceView->Release();
			irradiance_texture.mpTextureResourceView = NULL;

		}

		if(delta_irradiance_texture.mpTexture2D)			
		{
			delta_irradiance_texture.mpTexture2D->Release();
			delta_irradiance_texture.mpTexture2D = NULL;
			
			delta_irradiance_texture.mpRenderTargetView->Release();
			delta_irradiance_texture.mpRenderTargetView = NULL;
			
			delta_irradiance_texture.mpTextureResourceView->Release();
			delta_irradiance_texture.mpTextureResourceView = NULL;

			
		}
		if(scattering_texture.mpTexture3D)			
		{
			scattering_texture.mpTexture3D->Release();
			scattering_texture.mpTexture3D = NULL;

			scattering_texture.mpRenderTargetView->Release();
			scattering_texture.mpRenderTargetView = NULL;

			scattering_texture.mpTextureResourceView->Release();
			scattering_texture.mpTextureResourceView = NULL;

		}

		if(single_mie_scattering_texture.mpTexture3D)		
		{
			single_mie_scattering_texture.mpTexture3D->Release();
			single_mie_scattering_texture.mpTexture3D = NULL;
			
			single_mie_scattering_texture.mpRenderTargetView->Release();
			single_mie_scattering_texture.mpRenderTargetView = NULL;

			single_mie_scattering_texture.mpTextureResourceView->Release();
			single_mie_scattering_texture.mpTextureResourceView = NULL;

		}


		if(delta_rayleigh_scattering_texture.mpTexture3D)	
		{
			delta_rayleigh_scattering_texture.mpTexture3D->Release();
			delta_rayleigh_scattering_texture.mpTexture3D = NULL;
			
			delta_rayleigh_scattering_texture.mpRenderTargetView->Release();
			delta_rayleigh_scattering_texture.mpRenderTargetView = NULL;

			delta_rayleigh_scattering_texture.mpTextureResourceView->Release();
			delta_rayleigh_scattering_texture.mpTextureResourceView = NULL;

		}



		if(delta_mie_scattering_texture.mpTexture3D)	
		{
			delta_mie_scattering_texture.mpTexture3D->Release();
			delta_mie_scattering_texture.mpTexture3D = NULL;
			
			delta_mie_scattering_texture.mpRenderTargetView->Release();
			delta_mie_scattering_texture.mpRenderTargetView = NULL;

			delta_mie_scattering_texture.mpTextureResourceView->Release();
			delta_mie_scattering_texture.mpTextureResourceView = NULL;

		}

		if(delta_scattering_density_texture.mpTexture3D)	
		{
			delta_scattering_density_texture.mpTexture3D->Release();
			delta_scattering_density_texture.mpTexture3D = NULL;

			delta_scattering_density_texture.mpRenderTargetView->Release();
			delta_scattering_density_texture.mpRenderTargetView = NULL;

			delta_scattering_density_texture.mpTextureResourceView->Release();
			delta_scattering_density_texture.mpTextureResourceView = NULL;

		}

		if(test_irradiance_texture.mpTexture2D)
		{
			test_irradiance_texture.mpTexture2D->Release();
			test_irradiance_texture.mpTexture2D = NULL;

			test_irradiance_texture.mpRenderTargetView->Release();
			test_irradiance_texture.mpRenderTargetView = NULL;

			test_irradiance_texture.mpTextureResourceView->Release();
			test_irradiance_texture.mpTextureResourceView = NULL;

		}

		if(test_delta_irradiance_texture.mpTexture2D)
		{
			test_delta_irradiance_texture.mpTexture2D->Release();
			test_delta_irradiance_texture.mpTexture2D = NULL;
			
			test_delta_irradiance_texture.mpRenderTargetView->Release();
			test_delta_irradiance_texture.mpRenderTargetView = NULL;

			test_delta_irradiance_texture.mpTextureResourceView->Release();
			test_delta_irradiance_texture.mpTextureResourceView = NULL;

		}
	}

}






