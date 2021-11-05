#pragma once

static const float PI = 3.14159265358979323846f;

typedef double3 scalar3;

struct DensityProfileLayer
{
	scalar width;
	scalar exp_term;
	scalar exp_scale;
	scalar linear_term;
	scalar constant_term;
};

struct DensityProfile
{
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters
{
	scalar3 solar_irradiance;
	scalar sun_angular_radius;
	scalar bottom_radius;
	scalar top_radius;
	DensityProfile rayleigh_density;
	scalar3 rayleigh_scattering;
	DensityProfile mie_density;
	scalar3 mie_scattering;
	scalar3 mie_extinction;
	scalar mie_phase_function_g;
	DensityProfile absorption_density;
	scalar3 absorption_extinction;
	scalar3 ground_albedo;
	scalar mu_s_min;
};
