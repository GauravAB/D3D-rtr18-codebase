#include "Atmosphere.h"
#include "common.h"
#include "renderer.h"
#include <cmath>
#include <assert.h>
#include <cstring>



typedef double scalar;
typedef double3 scalar3;

#include "atmosphere_definations.h"



namespace atmosphere
{

	ID3D11VertexShader* atmosphere_demo_vs;
	ID3D11PixelShader* atmosphere_demo_ps;

	ID3D11VertexShader* precompute_vertexshader;

	//precomputation in pixel shader
	ID3D11PixelShader* precompute_pixelshader_transmittance;
	ID3D11PixelShader* precompute_pixelshader_delta_irradiance;
	ID3D11PixelShader* precompute_pixelshader_single_scattering;
	ID3D11PixelShader* precompute_pixelshader_scattering_density;
	ID3D11PixelShader* precompute_pixelshader_indirect_irradiance;
	ID3D11PixelShader* precompute_pixelshader_multiple_scaterring;
	

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

	double a_wavelengths[num_lambda_slices];
	double a_solar_irradiance_coeffs[num_lambda_slices];
	double a_rayleigh_scattering_coeffs[num_lambda_slices];
	double a_mie_scattering_coeffs[num_lambda_slices];
	double a_mie_extinction_coeffs[num_lambda_slices];
	double a_absorption_extinction_coeffs[num_lambda_slices];
	double a_ground_albedo_coeffs[num_lambda_slices];

	struct PrecomputeConstantBuffer 
	{
		struct 
		{
			XMFLOAT4X4 luminance_from_radiance;
			int scattering_order;
			int test;
			float _pad[2];
		} data;

		ID3D11Buffer *pPrecomputeConstantBuffer = nullptr;
	
	} precompute_cb = {};

	AtmosphereParameters atmosphere_parameters;
	
	double sun_k_r, sun_k_g, sun_k_b;
	double sky_k_r, sky_k_g, sun_k_b;


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


	void create_transmittance_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext , FILE* pFile)
	{
		precompute_pixelshader_transmittance->Release();
		precompute_pixelshader_transmittance = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_transmittance, L"pixelShaderTransmittance.hlsl", pFile);
	}

	void create_delta_irradiance_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		precompute_pixelshader_delta_irradiance->Release();
		precompute_pixelshader_delta_irradiance = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_delta_irradiance, L"pixelShaderDeltaIrradiance.hlsl", pFile);
	}

	void create_precompute_pixelshader_single_scattering_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		precompute_pixelshader_single_scattering->Release();
		precompute_pixelshader_single_scattering = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_single_scattering, L"pixelShaderSingleScattering.hlsl", pFile);
	}

	

	void create_precompute_pixelshader_scattering_density(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		precompute_pixelshader_scattering_density->Release();
		precompute_pixelshader_scattering_density = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_scattering_density, L"pixelShaderScatteringDensity.hlsl", pFile);
	}


	void create_precompute_pixelshader_indirect_irradiance(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		precompute_pixelshader_indirect_irradiance->Release();
		precompute_pixelshader_indirect_irradiance = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_indirect_irradiance, L"pixelShaderIndirectIrradiance.hlsl", pFile);
	}

	void create_precompute_pixelshader_multiple_scaterring(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, FILE* pFile)
	{
		precompute_pixelshader_multiple_scaterring->Release();
		precompute_pixelshader_multiple_scaterring = NULL;

		compile_and_create_pixel_shader(pID3D11Device, pID3D11DeviceContext, &precompute_pixelshader_multiple_scaterring, L"pixelShaderMultipleScattering.hlsl", pFile);
	}



	void precompute(ID3D11DeviceContext* pID3D11DeviceContext)
	{

		{
			//Precompute Transmittance
			D3D11_VIEWPORT viewport = { 0.0,0.0,TRANSMITTANCE_TEXTURE_WIDTH,TRANSMITTANCE_TEXTURE_HEIGHT,0.0,1.0 };
			pID3D11DeviceContext->RSSetViewports(1, &viewport);
			pID3D11DeviceContext->PSSetShader(precompute_pixelshader_transmittance, 0, 0);
			ID3D11RenderTargetView* a_rtv = transmittance_texture.mpRenderTargetView;
			pID3D11DeviceContext->OMSetRenderTargets(1, &a_rtv, nullptr);
			pID3D11DeviceContext->Draw(4, 0);
			ID3D11RenderTargetView* const a_null_rtv = {};
			//detach the rtv
			pID3D11DeviceContext->OMSetRenderTargets(1, &a_null_rtv, nullptr);
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
			ID3D11RenderTargetView* a_rtv[] = { delta_rayleigh_scattering_texture.mpRenderTargetView,
												delta_mie_scattering_texture.mpRenderTargetView,
												scattering_texture.mpRenderTargetView,
												single_mie_scattering_texture.mpRenderTargetView };
			pID3D11DeviceContext->OMSetRenderTargets(4, a_rtv, nullptr);
			pID3D11DeviceContext->Draw(4, 0);
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

					precompute_cb.data.scattering_order = scattering_order;
					renderer::update_cb(pID3D11DeviceContext, precompute_cb.pPrecomputeConstantBuffer, &precompute_cb.data,sizeof(precompute_cb));


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
					precompute_cb.data.scattering_order = scattering_order - 1;
					renderer::update_cb(pID3D11DeviceContext, precompute_cb.pPrecomputeConstantBuffer, &precompute_cb.data, sizeof(precompute_cb));

					D3D11_VIEWPORT viewport{ 0.0,0.0,IRRADIANCE_TEXTURE_WIDTH , IRRADIANCE_TEXTURE_HEIGHT , 0.0 , 1.0 };
					pID3D11DeviceContext->RSSetViewports(1, &viewport);
					ID3D11ShaderResourceView* a_srvs[] = { delta_rayleigh_scattering_texture.mpTextureResourceView,
														   delta_mie_scattering_texture.getAddressOfSRV,
														   delta_rayleigh_scattering_texture.mpTextureResourceView
					};
					pID3D11DeviceContext->PSSetShaderResources(0, 3, a_srvs);
					pID3D11DeviceContext->PSSetShader(precompute_pixelshader_indirect_irradiance, 0, 0);

					ID3D11RenderTargetView* a_rtv[] = { delta_irradiance_texture.mpRenderTargetView,
														irradiance_texture.mpRenderTargetView,
					};
					pID3D11DeviceContext->OMSetRenderTargets(2, a_rtv, nullptr);
					pID3D11DeviceContext->OMSetBlendState(nullptr, NULL, 0xffffffff);
					pID3D11DeviceContext->Draw(4, 0);
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
					//multiple draws for multiple scatter
					pID3D11DeviceContext->DrawInstanced(4, SCATTERING_TEXTURE_DEPTH, 0, 0);
					ID3D11RenderTargetView* const a_null_rtv[2] = {};
					//detach the rtv
					pID3D11DeviceContext->OMSetRenderTargets(2, a_null_rtv, nullptr);
					ID3D11ShaderResourceView* const a_null_srv[2] = { };
					pID3D11DeviceContext->PSSetShaderResources(0, 2, a_srv);
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
		
	}


	void init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext)
	{
		//create constant buffer

		XMMATRIX identity = XMMatrixIdentity();
		XMStoreFloat4x4(&precompute_cb.data.luminance_from_radiance, identity);
		renderer::create_cb(pID3D11Device,precompute_cb.pPrecomputeConstantBuffer, &precompute_cb.data, sizeof(precompute_cb.data));


		//set precomputed luminance value here
	}



}
