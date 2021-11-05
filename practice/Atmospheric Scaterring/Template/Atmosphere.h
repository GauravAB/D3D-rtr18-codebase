#pragma once
#include "common.h"
#include "shaderCompiler.h"

namespace renderer 
{
	struct Texture2D;
	struct Texture3D;
}

struct double3 { double x, y, z; };

constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

//rayleighs constant
constexpr int SCATTERING_TEXTURE_R_SIZE = 32;
//cosine of view zenith
constexpr int SCATTERING_TEXTURE_MU_SIZE = 128;
//consine of sun zenith
constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;
//sun angle
constexpr int SCATTERING_TEXTURE_NU_SIZE = 8;

//scatter texture
constexpr int SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
constexpr int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

//multiple scatter
constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

//wavelengths for red , green , blue
constexpr double kLambdaR = 680.0;
constexpr double kLambdaG = 550.0;
constexpr double kLambdaB = 440.0;

//some more constants from bruneton
constexpr double kPi = 3.1415926;
constexpr double kSunAngularRadius = 0.00935 / 2.0;
constexpr double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
constexpr double kLengthUnitInMeters = 1000.0;


namespace atmosphere
{

	void init(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext);

	ID3D11VertexShader* get_atmosphere_demo_vs();
	ID3D11PixelShader* get_atmosphere_demo_ps();
	renderer::Texture2D get_transmittance_texture();
	renderer::Texture3D get_scattering_texture();
	renderer::Texture3D get_single_mie_scattering_texture();
	renderer::Texture2D get_irradiance_texture();
	
	//void compute_white_point(double* p_white_point_r, double* p_white_point_g, double* p_white_point_b);

//	float compute_luminance_from_radiance_coeff(double lambda, double delta_lambda, int component);

}