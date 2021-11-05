#include "common.h"



void compile_and_create_vertex_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* ppID3D11DeviceContext, ID3D11VertexShader**ppID3D11VertexShader, ID3D11InputLayout** ppID3D11InputLayout,const wchar_t* shader_file_name , FILE *gpFile)
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
			fopen_s(&gpFile, "log.txt", "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for vertex shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;

		}
	}
	else
	{
		trace("ShaderCompiler::D3DCompileFromFile() succeeded for vertex shader. \n", gpFile);
	}

	hr = pID3D11Device->CreateVertexShader(pID3DBlob_ShaderCode->GetBufferPointer(), pID3DBlob_ShaderCode->GetBufferSize(), NULL, ppID3D11VertexShader);

	if (FAILED(hr))
	{
		trace("CreateVertexShader() failed.\n",gpFile);
	}
	else
	{
		trace("CreateVertexShader() succeeded.\n", gpFile);

	}

	//pID3D11DeviceContext->VSSetShader(pID3D11VertexShader, 0, 0);


	D3D11_INPUT_ELEMENT_DESC inputElementDesc[4];

	inputElementDesc[0].SemanticName = "POSITION";							//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[0].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//Format defines the data type to be used for this element.
	inputElementDesc[0].InputSlot = 0;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[0].AlignedByteOffset = 0;								//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[0].InstanceDataStepRate = 0;							//This field is used for instancing.

	inputElementDesc[1].SemanticName = "COLOR";							//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[1].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[1].Format = DXGI_FORMAT_R8G8B8A8_UNORM;				//Format defines the data type to be used for this element.
	inputElementDesc[1].InputSlot = 0;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[1].AlignedByteOffset = 0;								//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[1].InstanceDataStepRate = 0;							//This field is used for instancing.


	inputElementDesc[2].SemanticName = "TEXCOORD";								//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[2].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;				//Format defines the data type to be used for this element.
	inputElementDesc[2].InputSlot = 1;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[2].AlignedByteOffset = 0;	//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[2].InstanceDataStepRate = 0;							//This field is used for instancing.


	inputElementDesc[3].SemanticName = "NORMAL";								//Semantic name is a string containing a word that describes the nature or purpose (or semantics) of this element.
	inputElementDesc[3].SemanticIndex = 0;									//Instead of using semantic names that have numbers appended, such as "COLOR0" and "COLOR1", the two elements can share a single semantic name, "COLOR", with different semantic indices 0 and 1.
	inputElementDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//Format defines the data type to be used for this element.
	inputElementDesc[3].InputSlot = 1;										//As mentioned previously, a Direct3D 11 application passes vertex data to the GPU via the use of vertex buffer.
	inputElementDesc[3].AlignedByteOffset = 0;	//The AlignedByteOffset field tells the GPU the memory location to start fetching the data for this element.
	inputElementDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex data or instance data.
	inputElementDesc[3].InstanceDataStepRate = 0;							//This field is used for instancing.


	hr = pID3D11Device->CreateInputLayout
	(
		inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_ShaderCode->GetBufferPointer(),	//vertex shader signature
		pID3DBlob_ShaderCode->GetBufferSize(),
		ppID3D11InputLayout
	);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateInputLayout() failed.\n", gpFile);
	
	}
	else
	{
		trace("gpID3D11Device::CreateInputLayout() Succeeded.\n", gpFile);
	}

	//ppID3D11DeviceContext->IASetInputLayout(ppID3D11InputLayout);
	pID3DBlob_ShaderCode->Release();
	pID3DBlob_ShaderCode = NULL;
}


void compile_and_create_pixel_shader(ID3D11Device* pID3D11Device, ID3D11DeviceContext* pID3D11DeviceContext, ID3D11PixelShader** ppID3D11PixelShader, const wchar_t* shader_file_name, FILE* gpFile)
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
		0,
		0,
		&pID3DBlob_ShaderCode,
		&pID3DBlob_Error);


	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, "log.txt", "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for pixel shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;

		}
	}
	else
	{
		trace("ShaderCompiler::D3DCompileFromFile() succeeded for pixel shader. \n", gpFile);
	}

	hr = pID3D11Device->CreatePixelShader(pID3DBlob_ShaderCode->GetBufferPointer(), pID3DBlob_ShaderCode->GetBufferSize(), NULL,ppID3D11PixelShader);

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
