#pragma once
#include "shaderCompiler.h"



void compile_and_create_vertex_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11VertexShader** pID3D11VertexShader, ID3D11InputLayout** pID3D11InputLayout, const wchar_t* shader_file_name , FILE *gpFile)
{
	HRESULT hr;
	ID3DBlob* pID3DBlob_ShaderCode = NULL;
	ID3DBlob* pID3DBlob_Error = NULL;


	hr = D3DCompileFromFile(
		shader_file_name,
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_ShaderCode,
		&pID3DBlob_Error);


	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, "Log.txt", "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for vertex shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;

		}
	}
	else
	{
		trace("D3DCompile() succeeded for vertex shader. \n", gpFile);
	}

	hr = pID3D11Device->CreateVertexShader(pID3DBlob_ShaderCode->GetBufferPointer(), pID3DBlob_ShaderCode->GetBufferSize(), NULL, pID3D11VertexShader);

	if (FAILED(hr))
	{
		trace("CreateVertexShader() failed.\n",gpFile);
	}
	else
	{
		trace("CreateVertexShader() succeeded.\n", gpFile);

	}

	//pID3D11DeviceContext->VSSetShader(*pID3D11VertexShader, 0, 0);


	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];

	inputElementDesc[0].SemanticName = "POSITION";							//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[0].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//Format defines the data type to be used for this element.
	inputElementDesc[0].InputSlot = 0;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[0].AlignedByteOffset = 0;								//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[0].InstanceDataStepRate = 0;							//This field is used for instancing.

	inputElementDesc[1].SemanticName = "NORMAL";								//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[1].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//Format defines the data type to be used for this element.
	inputElementDesc[1].InputSlot = 1;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[1].AlignedByteOffset = 0;	//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[1].InstanceDataStepRate = 0;							//This field is used for instancing.

	
	hr = pID3D11Device->CreateInputLayout
	(
		inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_ShaderCode->GetBufferPointer(),	//vertex shader signature
		pID3DBlob_ShaderCode->GetBufferSize(),
		pID3D11InputLayout
	);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateInputLayout() failed.\n", gpFile);
	
	}
	else
	{
		trace("gpID3D11Device::CreateInputLayout() Succeeded.\n", gpFile);
	}

	///pID3D11DeviceContext->IASetInputLayout(*pID3D11InputLayout);
	pID3DBlob_ShaderCode->Release();
	pID3DBlob_ShaderCode = NULL;
}


void compile_and_create_pixel_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11PixelShader** pID3D11PixelShader, const wchar_t* shader_file_name, FILE* gpFile)
{
	HRESULT hr;
	ID3DBlob* pID3DBlob_ShaderCode = NULL;
	ID3DBlob* pID3DBlob_Error = NULL;


	hr = D3DCompileFromFile(
		shader_file_name,
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION,
		0,
		&pID3DBlob_ShaderCode,
		&pID3DBlob_Error);


	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, "Log.txt", "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for pixel shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;

		}
	}
	else
	{
		trace("D3DCompile() succeeded for pixel shader. \n", gpFile);
	}

	hr = pID3D11Device->CreatePixelShader(pID3DBlob_ShaderCode->GetBufferPointer(), pID3DBlob_ShaderCode->GetBufferSize(), NULL, pID3D11PixelShader);

	if (FAILED(hr))
	{
		trace("CreatePixelShader() failed.\n", gpFile);
	}
	else
	{
		trace("CreatePixelShader() succeeded.\n", gpFile);

	}

//	pID3D11DeviceContext->PSSetShader(pID3D11PixelShader, 0, 0);
	pID3DBlob_ShaderCode->Release();
	pID3DBlob_ShaderCode = NULL;
}




void compile_and_create_geometry_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11GeometryShader** pID3D11GeometryShader, const wchar_t* shader_file_name, FILE* gpFile)
{
	HRESULT hr;

	ID3DBlob* pID3DBlob_ShaderCode = NULL;
	ID3DBlob* pID3DBlob_Error = NULL;


	hr = D3DCompileFromFile(
		shader_file_name,
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"gs_5_0",
		D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION,
		0,
		&pID3DBlob_ShaderCode,
		&pID3DBlob_Error);


	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, "Log.txt", "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for geometry shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;

		}
	}
	else
	{
		trace("D3DCompile() succeeded for geometry shader. \n", gpFile);
	}

	hr = pID3D11Device->CreateGeometryShader(pID3DBlob_ShaderCode->GetBufferPointer(), pID3DBlob_ShaderCode->GetBufferSize(), NULL, pID3D11GeometryShader);

	if (FAILED(hr))
	{
		trace("CreateGeometryShader() failed.\n", gpFile);
	}
	else
	{
		trace("CreateGeometryShader() succeeded.\n", gpFile);

	}


	pID3DBlob_ShaderCode->Release();
	pID3DBlob_ShaderCode = NULL;
}
