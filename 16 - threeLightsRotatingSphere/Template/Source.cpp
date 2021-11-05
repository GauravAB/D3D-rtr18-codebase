#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Sphere.h"





#pragma warning(disable:4838)
#include "XNAMath/xnamath.h"




#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"sphere.lib")



struct CBUFFER
{
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR lPos;
};

//phong lighting components in a cbuffer
struct CPHONGBUFFER
{
	XMVECTOR La;	//global ambient color of incoming light
	XMVECTOR Ld;	//light diffuse color
	XMVECTOR Ls;	//light specular color

	XMVECTOR Ka;	//material's ambient reflectance
	XMVECTOR Kd;	//material's diffuse color 
	XMVECTOR Ks;	//material's specular color

	float shininess;
	unsigned int isLightKeyPressed;
};

//mesh variables
unsigned int toggleKey;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gSphereVertices;
unsigned int gSphereElements;


XMMATRIX gPerspectiveProjectionMatrix;

float gClearColor[4];


HWND ghwnd;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbIsEscapeKeyPressed = false;
bool gbActiveWindow = false;
bool gbFullScreen = false;

IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

//View resources
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

//Buffers
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBufferPhong = NULL;

ID3D11Buffer* gpID3D11Buffer_SpherePosition = NULL;
ID3D11Buffer* gpID3D11Buffer_SphereNormal = NULL;
ID3D11Buffer* gpID3D11Buffer_IndexBuffer = NULL;


ID3D11InputLayout *gpID3D11InputLayout = NULL;


ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
float gAngle;

float lightPosition[] = { 0.0f,0.0f,0.0f,0.0f };



FILE *gpFile = NULL;
char gszLogFileName[] = "log.txt";


void trace(const char* txt)
{
	
	fopen_s(&gpFile, gszLogFileName, "a+");
	fprintf_s(gpFile, txt);
	fclose(gpFile);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nCmdShow)
{
	HRESULT initialize(void);
	void display(void);
	void update(void);

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("my perspective D3D window");
	bool bDone = false;

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = NULL;
	wndclass.cbWndExtra = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName, TEXT("perspective"), WS_OVERLAPPEDWINDOW, 0, 0, WIN_WIDTH, WIN_HEIGHT,0, 0, hInstance,0);

	ghwnd = hwnd;

	ShowWindow(hwnd, nCmdShow);
//	UpdateWindow(hwnd);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Initialize 3D
	HRESULT hr;
	hr = initialize();


	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else  //game loop
		{
			if (gbActiveWindow)
			{
				if (gbIsEscapeKeyPressed)
				{
					bDone = true;
				}
			}
			update();
			display();
		}
	}
	
	return ((int)(msg.wParam));
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullScreen(void);
	HRESULT resize(int, int);

	HRESULT hr;

	switch (uMsg)
	{
		
		case WM_SIZE:
			if (gpID3D11DeviceContext)
			{
				hr = resize(LOWORD(lParam), HIWORD(lParam));
				if (FAILED(hr))
				{
					fopen_s(&gpFile, gszLogFileName, "a+");
					fprintf_s(gpFile, "resize() Failed. \n");
					fclose(gpFile);
					return(hr);
				}
				else
				{
					fopen_s(&gpFile, gszLogFileName, "a+");
					fprintf_s(gpFile, "resize() Suceeded.");
					fclose(gpFile);
				}
			}
			break;

		case WM_ACTIVATE:
			if (HIWORD(wParam) == 0)
			{
				gbActiveWindow = true;
			}
			else
			{
				gbActiveWindow = false;
			}
			break;

		case WM_KEYDOWN:
			switch (wParam)
			{
				case 'l':
				case 'L':
					if (toggleKey)
					{
						toggleKey = 0;
					}
					else
					{
						toggleKey = 1;
					}
					break;
				case 'f':
				case 'F':
					if (gbFullScreen == false)
					{
						ToggleFullScreen();
						gbFullScreen = true;
					}
					else
					{
						ToggleFullScreen();
						gbFullScreen = false;
					}
					break;
				case VK_ESCAPE:
					gbIsEscapeKeyPressed = true;
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);

			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}


HRESULT initialize(void)
{
	//function signatures
	void uninitialize(void);
	HRESULT resize(int, int);

	//variable declaration
	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;

	//sequence is according to priority
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE,D3D_DRIVER_TYPE_WARP,D3D_DRIVER_TYPE_REFERENCE };
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;

	//default lowest
	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	//code
	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	//first create swap chain
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			d3dDriverType,
			NULL,
			createDeviceFlags,
			&d3dFeatureLevel_required,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDesc,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevel_acquired,
			&gpID3D11DeviceContext);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		trace("D3D11CreateDeviceAndSwapChain() failed. \n");
		return hr;
	}
	else
	{
		trace("D3D11CreateDeviceSwapChain() Succeeded. \n");
		trace("The Chosen Driver is of ");

		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			trace("Hardware Type. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			trace("SoftwareType. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			trace("Reference Type. \n");
		}
		else
		{
			trace("Unknown Type. \n");
		}

		trace("The supported highest feature level is");

		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			trace("11.0\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			trace("10.1\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			trace("Unknown.\n");
		}
	}


	//Initialize shaders , input layouts , constant buffers

	const char* vertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
			"float4x4 worldMatrix;"\
			"float4x4 viewMatrix;"\
			"float4x4 projectionMatrix;"\
			"float4 lightPos;"\
		"}"\
		"struct vertex_output"\
		"{"\
			"float4 position:SV_POSITION;"\
			"float3 tNorm:NORMAL0;"\
			"float3 lightDirection:NORMAL1;"\
			"float3 viewVector:NORMAL2;"\
		"};"\
		"vertex_output main(float4 pos:POSITION , float4 norm:NORMAL)"\
		"{"\
			"vertex_output output;"\
			"float4 eyeCoordinates = mul(viewMatrix,mul(worldMatrix,pos));"\
			"output.tNorm = mul((float3x3)mul(viewMatrix,worldMatrix),(float3)norm);"\
			"output.lightDirection = (float3)(lightPos - eyeCoordinates);"\
			"output.viewVector = (float3)(-eyeCoordinates);"\
			"output.position = mul(projectionMatrix,eyeCoordinates);"\
			"return(output);"\
		"}"; \

		ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Failed for vertex shader %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		trace("D3DCompile() succeeded for vertex shader. \n");
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);

	if (FAILED(hr))
	{
		trace("CreateVertexShader() failed.\n");
	}
	else
	{
		trace("CreateVertexShader() succeeded.\n");

	}

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

	const char* pixelShaderSourceCode =
		"struct vertex_output"\
		"{"\
			"float4 position: SV_POSITION;"\
			"float3 tNorm: NORMAL0;"\
			"float3 lightDirection:NORMAL1;"\
			"float3 viewVector:NORMAL2;"\
		"};"\

		"cbuffer phongBuffer"\
		"{"\
			"float4 la;"\
			"float4 ld;"\
			"float4 ls;"\
			"float4 ka;"\
			"float4 kd;"\
			"float4 ks;"\
			"float shininess;"\
			"uint togglekey;" \

		"}"\

		"float4 main(float4 position:SV_POSITION,vertex_output input):SV_TARGET"\
		"{"\
			"float3 normal = normalize(input.tNorm);"\
			"float3 source = normalize(input.lightDirection);"\
			"float4 ambient = ka * la;"\
			"float4 diffuse = kd * ld* max(dot(normal,source),0.0);"\
			"float3 reflectionVector = reflect(-source,normal);"\
			"float spec = pow(max(dot(reflectionVector,normalize(input.viewVector)),0.0),shininess);"\
			"float4 specular = ls * ks * spec;"\
			"float4 color = float4(1.0,1.0,1.0,1.0);"\
			"if(togglekey == 0)"\
			"{"\
			
			"}"\
			"else"\
			"{"\
				"color = ambient + diffuse + specular;"\
			"}"\
			"return (color);"\
		"}";


	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() failed for pixel shader: %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return (hr);
		}
	}
	else
	{
		trace("D3DCompile() Succeeded for pixel shader. \n");
	}


	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(), pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);

	if (SUCCEEDED(hr))
	{
		trace("createPixelShader() succeeded. \n");
	}

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);

	gSphereVertices = getNumberOfSphereVertices();
	gSphereElements = getNumberOfSphereElements();


	//create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//create vertex buffer
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_SpherePosition);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere position buffer");
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere position buffer");
	}


	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	
	gpID3D11DeviceContext->Map(gpID3D11Buffer_SpherePosition, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_vertices, sizeof(float) * ARRAYSIZE(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_SpherePosition, NULL);

	
	//normal buffer now
	
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_normals);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_SphereNormal);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere normal buffer");
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere normal buffer");
	}





	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_SphereNormal, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_normals, sizeof(float) * ARRAYSIZE(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_SphereNormal, NULL);

	

	//Index Buffer
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(short) * gSphereElements;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//Create Vertex Buffer

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_IndexBuffer);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere index buffer");
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere index buffer");
	}


	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_elements, sizeof(short)*gSphereElements);
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL);




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


	hr = gpID3D11Device->CreateInputLayout
	(
		inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),	//vertex shader signature
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout
	);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateInputLayout() failed.\n");
		return (hr);
	}
	else
	{
		trace("gpID3D11Device::CreateInputLayout() Succeeded.\n");
	}

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	

	//define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateBuffer() failed. \n");
		return hr;
	}
	else
	{
		trace("gpID3D11Device::CreateBuffer() Succeeded. \n");
	}

	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	//phong buffer

	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CPHONGBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBufferPhong);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateBuffer() failed. \n");
		return hr;
	}
	else
	{
		trace("gpID3D11Device::CreateBuffer() Succeeded. \n");
	}

	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBufferPhong);


	//set rasterization state 
	//In D3D , backface culling is ON by default
	//means while rotation , geometry will cull from one side
	//so lets set culling off

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	hr = gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);

	if (FAILED(hr))
	{
		trace("ID3D11Device::CreateRasterizerState() failed for culling. \n");
	}
	else
	{
		trace("ID3D11Device::CreateRasterizerState() succeeded for culling. \n");
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//d3d clear color
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 0.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	//call resize for the first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
	{
		trace("resize() failed. \n");
	}
	else
	{
		trace("resize() succeeded. \n");
	}

	return (S_OK);
}


HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;

	//free any size dependent resources

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//again get back buffer from swap chain
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);

	//again	get render target view from d3d11 device using above back buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);

	if (FAILED(hr))
	{
		trace("ID3D11Device::CreateRenderTargetView() , failed. \n");
	}
	else
	{
		trace("ID3D11Device::CreateRenderTargetView() succeeded.\n");
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	//create depth stencil buffer
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;	//can be upto 4
	textureDesc.SampleDesc.Quality = 0;	//if above 4 , then it is 1
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer;
	gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	//create depth stencil view from above depth stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);

	if (FAILED(hr))
	{
		trace("ID3D11Device::CreateDepthStencilView() Failed. \n");
	}
	else
	{
		trace("ID3D11Device::CreateDepthStencilView() Success. \n");
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	

	//set render target view as target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	//set viewport
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	//set perspective matrix
	if (width >= height)
	{
		gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	}
	else
	{
		gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)height / (float)width, 0.1f, 100.0f);

	}
	return (hr);
}

void update(void)
{
	gAngle += 0.001f;

	if (gAngle >= 360.0f)
	{
		gAngle = 0.0f;
	}
}

void display(void)
{
	//create render target view for choosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	//clear depth and stencil view to 1.0
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//select vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_SpherePosition, &stride, &offset);
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_SphereNormal, &stride, &offset);

	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer,DXGI_FORMAT_R16_UINT,0);


	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();

	//transformations
	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);

	CBUFFER constantBuffer;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;

	XMMATRIX rotationMatrix = XMMatrixRotationY(gAngle*3.0f);
	worldMatrix = XMMatrixMultiply(rotationMatrix,worldMatrix);
	constantBuffer.WorldMatrix = worldMatrix;

	XMVECTOR lightPosition = XMVectorSet(sinf(gAngle),0.0f,2.0f+cosf(gAngle),0.0f);

	//lightPosition = XMVector4Transform(lightPosition, rotationMatrix);

	constantBuffer.lPos = lightPosition;




	CPHONGBUFFER phongBuffer;

	phongBuffer.La = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	phongBuffer.Ld = XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f);
	phongBuffer.Ls = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	phongBuffer.Ka = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	phongBuffer.Kd = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	phongBuffer.Ks = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	phongBuffer.isLightKeyPressed = toggleKey;
	phongBuffer.shininess = 20.0f;


	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBufferPhong, 0, NULL, &phongBuffer, 0, 0);
	

	gpID3D11DeviceContext->DrawIndexed(gSphereElements, 0, 0);
	

	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);  //first parameter 0 - The presentation occurs immediately, there is no synchronization.
									  //second parameter 0 - An integer value that contains swap-chain presentation options. 
}


void uninitialize(void)
{

	if (gpID3D11Buffer_SpherePosition)
	{
		gpID3D11Buffer_SpherePosition->Release();
		gpID3D11Buffer_SpherePosition = NULL;
	}


	if (gpID3D11Buffer_SphereNormal)
	{
		gpID3D11Buffer_SphereNormal->Release();
		gpID3D11Buffer_SphereNormal = NULL;
	}


	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}

	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}


	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}
											
	if (gpFile)
	{
		trace("uninitialize() succeeded. \n");
		trace("Log File is successfully closed.");
	}
}






