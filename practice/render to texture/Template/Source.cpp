#include "common.h"
#include "sphere.h"
#include "shaderCompiler.h"
#include "RenderToTexture.h"


#pragma warning(disable:4838)
#include "XNAMath/xnamath.h"




#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


void trace(const char* txt, FILE* gpFile)
{

	fopen_s(&gpFile, "log.txt", "a+");
	fprintf_s(gpFile, txt);
	fclose(gpFile);
}


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
unsigned int sphereVertices;
unsigned int sphereElements;
unsigned int toggleKey;



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

ID3D11VertexShader* gpID3D11VertexShaderDemo = NULL;
ID3D11PixelShader* gpID3D11PixelShaderDemo = NULL;

ID3D11InputLayout* gpID3D11VertexShaderInputLayout = NULL;
ID3D11InputLayout* gpID3D11VertexShaderDemoInputLayout = NULL;


ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

//View resources
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

//Buffers
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBufferPhong = NULL;

ID3D11Buffer* gpID3D11Buffer_SpherePosition = NULL;
ID3D11Buffer* gpID3D11Buffer_SphereNormal = NULL;
ID3D11Buffer* gpID3D11Buffer_IndexBuffer = NULL;
ID3D11Buffer* gpID3D11Buffer_CubeVertices = NULL;
ID3D11Buffer* gpID3D11Buffer_CubeTexCoords = NULL;


ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11SamplerState* gpID3D11SamplerState = NULL;


ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
float gAngle;

RenderToTexture* gpRenderToTexture = NULL;



FILE *gpFile = NULL;
char gszLogFileName[] = "log.txt";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nCmdShow)
{
	HRESULT initialize(void);
	void display(void);
	void update(void);

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("D3D11 Engine");
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

	fopen_s(&gpFile, gszLogFileName, "w");
	fprintf_s(gpFile, "log file for D3D11RenderEngine\n");
	fclose(gpFile);

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
		trace("D3D11CreateDeviceAndSwapChain() failed. \n",gpFile);
		return hr;
	}
	else
	{
		trace("D3D11CreateDeviceSwapChain() Succeeded. \n", gpFile);
		trace("The Chosen Driver is of ", gpFile);

		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			trace("Hardware Type. \n", gpFile);
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			trace("SoftwareType. \n", gpFile);
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			trace("Reference Type. \n", gpFile);
		}
		else
		{
			trace("Unknown Type. \n", gpFile);
		}

		trace("The supported highest feature level is", gpFile);

		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			trace("11.0\n", gpFile);
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			trace("10.1\n", gpFile);
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			trace("Unknown.\n", gpFile);
		}
	}


	//Initialize shaders , input layouts , constant buffers
	
	compile_and_create_vertex_shader(gpID3D11Device, gpID3D11DeviceContext, &gpID3D11VertexShader,&gpID3D11VertexShaderInputLayout, L"vertexShader.hlsl", gpFile);
	compile_and_create_pixel_shader(gpID3D11Device,gpID3D11DeviceContext, &gpID3D11PixelShader,L"pixelShader.hlsl",gpFile);


	compile_and_create_vertex_shader(gpID3D11Device, gpID3D11DeviceContext, &gpID3D11VertexShaderDemo, &gpID3D11VertexShaderDemoInputLayout, L"vertexShaderDemo.hlsl", gpFile);
	compile_and_create_pixel_shader(gpID3D11Device, gpID3D11DeviceContext, &gpID3D11PixelShaderDemo, L"pixelShaderDemo.hlsl", gpFile);
	


	//initialize struct sphere mesh
	mesh* sphere = (mesh*)malloc(sizeof(mesh));

	//fill in the pointers inside the struct
	makeSphere(sphere, 30, 30, 1.0);
	sphereVertices = sphere->numVertices;
	sphereElements = sphere->numElements;


	//create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * 3 * sphereVertices;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//create vertex buffer
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_SpherePosition);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere position buffer", gpFile);
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere position buffer", gpFile);
	}


	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	
	gpID3D11DeviceContext->Map(gpID3D11Buffer_SpherePosition, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere->verts, sizeof(float) * 3 * sphereVertices);
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_SpherePosition, NULL);

	
	//normal buffer now
	
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * 3 * sphereVertices;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_SphereNormal);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere normal buffer", gpFile);
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere normal buffer", gpFile);
	}





	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_SphereNormal, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere->norms, sizeof(float) * 3 * sphereVertices);
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_SphereNormal, NULL);

	

	//Index Buffer
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * sphere->numElements;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//Create Vertex Buffer

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_IndexBuffer);

	if (FAILED(hr))
	{
		trace("ID3D11Device: CreateBuffer() failed to create sphere index buffer", gpFile);
	}
	else
	{
		trace("ID3D11Device: CreateBuffer() succeeded to create sphere index buffer", gpFile);
	}


	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere->elements, sizeof(unsigned int) *  sphereElements);
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL);



	free(sphere->verts);
	free(sphere->norms);
	free(sphere->texCoords);
	free(sphere);




	float cube_vertices[] =
	{
		//top
	   //triangle 1
	   -1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	   -1.0f, 1.0f,-1.0f,
	   //triangle 2
	   -1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,

		//bottom
		//triangle 1
		1.0f,-1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
	   -1.0f,-1.0f,-1.0f,

	   //triangle 2
	   -1.0f,-1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
	   -1.0f,-1.0f, 1.0f,

	   //front
	   //triangle 1
	   -1.0f, 1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
	   -1.0f,-1.0f,-1.0f,

	   //triangle 2
	   -1.0f, -1.0f, -1.0f,
	   1.0f, 1.0f, -1.0f,
	   1.0f, -1.0f, -1.0f,

	   //back
	   //triangle 1
	   1.0f,-1.0f,1.0f,
	   1.0f,1.0f,1.0f,
	   -1.0f,-1.0f,1.0f,

	   //triangle 2
	   -1.0f,-1.0f,1.0f,
	   1.0f,1.0f,1.0f,
	   -1.0f,1.0f,1.0f,

	   //left
	   //triangle 1
	   -1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,

	   //triangle 2
	   -1.0f, -1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,

	   //right 
	   //triangle 1
	   1.0f, -1.0f, -1.0f,
	   1.0f,  1.0f, -1.0f,
	   1.0f, -1.0f,  1.0f,

	   //triangle 2
	   1.0f, -1.0f,  1.0f,
	   1.0f,  1.0f, -1.0f,
	   1.0f,  1.0f,  1.0f
	};


	float cube_texcoords[] =
	{
		//top
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,

		//bottom
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,

		//front
		1.0f,1.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		0.0f,0.0f,

		//back
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,

		//left
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,

		//right
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f
	};


	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(cube_vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//create vertex buffer
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_CubeVertices);

	if (FAILED(hr))
	{
		trace("ID3D11Device::CreateBuffer() failed for vertex buffer for Cube Vertices \n",gpFile);
	}
	else
	{
		trace("ID3D11Device::CreateBuffer() Succeeded for vertex buffer for Cube Vertices \n",gpFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_CubeVertices, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cube_vertices, sizeof(cube_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_CubeVertices, NULL);


	//colour for square
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(cube_texcoords);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//create vertex buffer
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_CubeTexCoords);

	if (FAILED(hr))
	{
		trace("ID3D11Device::CreateBuffer() failed for vertex buffer for square \n",gpFile);
	}
	else
	{
		trace("ID3D11Device::CreateBuffer() Succeeded for vertex buffer for square \n",gpFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_CubeTexCoords, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cube_texcoords, sizeof(cube_texcoords));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_CubeTexCoords, NULL);






	//define and set the constant buffer
	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer);

	if (FAILED(hr))
	{
		trace("gpID3D11Device::CreateBuffer() failed. \n", gpFile);
		return hr;
	}
	else
	{
		trace("gpID3D11Device::CreateBuffer() Succeeded. \n", gpFile);
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
		trace("gpID3D11Device::CreateBuffer() failed. \n", gpFile);
		return hr;
	}
	else
	{
		trace("gpID3D11Device::CreateBuffer() Succeeded. \n", gpFile);
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
		trace("ID3D11Device::CreateRasterizerState() failed for culling. \n", gpFile);
	}
	else
	{
		trace("ID3D11Device::CreateRasterizerState() succeeded for culling. \n", gpFile);
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	//d3d clear color
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 0.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();



	gpRenderToTexture = new RenderToTexture();

	if (!gpRenderToTexture)
	{
		trace("rendertotexture contructor failed",gpFile);
	}

	hr = gpRenderToTexture->Initialize(gpID3D11Device, WIN_WIDTH, WIN_HEIGHT);
	
	if (FAILED(hr))
	{
		trace("renderToTexture initialize failed",gpFile);
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState);

	if (FAILED(hr))
	{
		trace("Create Sampler state failed",gpFile);
	}
	else
	{
		trace("Sapler state for texture created successfully",gpFile);
	}




	//call resize for the first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
	{
		trace("resize() failed. \n", gpFile);
	}
	else
	{
		trace("resize() succeeded. \n", gpFile);
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
		trace("ID3D11Device::CreateRenderTargetView() , failed. \n", gpFile);
	}
	else
	{
		trace("ID3D11Device::CreateRenderTargetView() succeeded.\n", gpFile);
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
		trace("ID3D11Device::CreateDepthStencilView() Failed. \n", gpFile);
	}
	else
	{
		trace("ID3D11Device::CreateDepthStencilView() Success. \n", gpFile);
	}

	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	//Render Target Texture

	gpRenderToTexture->Uninitialize();
	//change w and h accordingly , for now keeping constant
	gpRenderToTexture->Initialize(gpID3D11Device, width, height);

	

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
}

void display(void)
{



	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
    gpID3D11DeviceContext->IASetInputLayout(gpID3D11VertexShaderInputLayout);

	//render to texture in first pass
	gpRenderToTexture->SetRenderTargetView(gpID3D11DeviceContext, gpID3D11DepthStencilView);
	gpRenderToTexture->ClearRenderTargetView(gpID3D11DeviceContext, gpID3D11DepthStencilView, 1.0f, 1.0f, 1.0f, 1.0f);
	
	
	//select vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	CPHONGBUFFER phongBuffer;
	CBUFFER constantBuffer;
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();


	
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_SpherePosition, &stride, &offset);
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_SphereNormal, &stride, &offset);
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//transformations
	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
	constantBuffer.lPos = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);

	phongBuffer.La = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	phongBuffer.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	phongBuffer.Ls = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	phongBuffer.Ka = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	phongBuffer.Kd = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	phongBuffer.Ks = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	phongBuffer.isLightKeyPressed = toggleKey;
	phongBuffer.shininess = 50.0f;


	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBufferPhong, 0, NULL, &phongBuffer, 0, 0);

	gpID3D11DeviceContext->DrawIndexed(sphereElements, 0, 0);
	




	//draw to the back buffer now
	//second pass
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);


	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShaderDemo, 0, 0);
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShaderDemo, 0, 0);
	gpID3D11DeviceContext->IASetInputLayout(gpID3D11VertexShaderDemoInputLayout);


	//create render target view for choosen color
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);
	//clear depth and stencil view to 1.0
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


	//select vertex buffer to display
	stride = sizeof(float) * 3;
	offset = 0;
	
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_CubeVertices, &stride, &offset);

	stride = sizeof(float) * 2;
	offset = 0;

	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_CubeTexCoords, &stride, &offset);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set resource and sampler for pixel shader
	ID3D11ShaderResourceView* temp = gpRenderToTexture->GetShaderResourceView();

	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &temp);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState);
	

	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();

	//transformations
	worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 6.0f);
	XMVECTOR axis = { 1.0,1.0,1.0,1.0 };

	rotationMatrix = XMMatrixRotationY(-gAngle);

	//worldMatrix = rotationMatrix * worldMatrix;

	//CBUFFER constantBuffer;
	constantBuffer.WorldMatrix = worldMatrix;
	constantBuffer.ViewMatrix = viewMatrix;
	constantBuffer.ProjectionMatrix = gPerspectiveProjectionMatrix;
	constantBuffer.lPos = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);


	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
	

	gpID3D11DeviceContext->Draw(36,0);

	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);  //first parameter 0 - The presentation occurs immediately, there is no synchronization.
									  //second parameter 0 - An integer value that contains swap-chain presentation options. 
}


void uninitialize(void)
{

	if (gpRenderToTexture)
	{
		gpRenderToTexture->Uninitialize();
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
		trace("uninitialize() succeeded. \n",gpFile);
		trace("Log File is successfully closed.",gpFile);
	}
}






