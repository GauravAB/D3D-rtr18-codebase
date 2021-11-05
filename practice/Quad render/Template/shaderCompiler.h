#pragma once
#include "common.h"

void compile_and_create_vertex_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11VertexShader* pID3D11VertexShader, ID3D11InputLayout* pID3D11InputLayout, const wchar_t* shader_file_name, FILE* gpFile);
void compile_and_create_pixel_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11PixelShader* pID3D11PixelShader, const wchar_t* shader_file_name, FILE* gpFile);



