#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "TextureLoaders/WICTextureLoader.h"



//#pragma warning(disable:4838)
//#include "XNAMath/xnamath.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

void trace(const char* txt, FILE* gpFile);
