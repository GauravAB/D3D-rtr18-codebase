#include <Windows.h>
#include <stdio.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include "XNAMath/xnamath.h"


#pragma warning(disable: 4838)
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

#define WIN32_LEAN_AND_MEAN
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

FILE *gpFile = NULL;
char gszLogFileName[] = "Log.txt";
HWND ghwnd;
bool gbActiveWindow = false;
bool gbIsEscapeKeyIsPressed = false;
IDXGISwapChain *gpIDXGISwapChain;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
//VBO
ID3D11Buffer *gpID3D11Buffer_VertexBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
//UBO
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};
								 
XMMATRIX gOrthographicProjectionMatrix;


float gClearColor[4];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	//function declaration
	HRESULT initialize(void);
	void uninitialize(void);
	void display(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("DIRECT 3D 11");
	bool bDone = false;

	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file cannot be created. exitting..."), TEXT("error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "File successfully opened\n");
		fclose(gpFile);
	}

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = NULL;
	wndclass.cbWndExtra = NULL;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;	
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	//registerclass
	RegisterClassEx(&wndclass);
	
	//create window
	hwnd = CreateWindow(szAppName, TEXT("Direct3D11 Window"), WS_OVERLAPPEDWINDOW, 100, 100, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);
	
	ghwnd = hwnd;
	
	ShowWindow(hwnd, nCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//initialize 3D
	HRESULT hr;
	hr = initialize();

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf(gpFile, "initialize() failed exitting now");
		DestroyWindow(hwnd);
		fclose(gpFile);
	}
	
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
		else
		{
			//render 
			display();

			if (gbActiveWindow == true)
			{
				if (gbIsEscapeKeyIsPressed == true)
				{


					bDone = true;
				}
			}
		}
	}
	
	uninitialize();
	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT resize(int, int);
	void ToggleFullScreen(void);
	void uninitialize(void);

	//HRESULT hr;

	switch (uiMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
	}

	return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

HRESULT initialize(void)
{

	//function signatures
	void unintialize(void);
	HRESULT resize(int, int);


	//variable declaration
	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	//sequence is according to priority
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
	
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;

	//default lowest

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;	//based upon d3dFeatureLevel_required

	//code
	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]); //calculate size of array

	//first create swap chain
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	dxgiSwapChainDesc.BufferCount = 1;
	/*
		typedef struct DXGI_MODE_DESC {
	  UINT                     Width;
	  UINT                     Height;
	  DXGI_RATIONAL            RefreshRate;
	  DXGI_FORMAT              Format;
	  DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
	  DXGI_MODE_SCALING        Scaling;
	} DXGI_MODE_DESC;
	*/
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghwnd;
	/*
		typedef struct DXGI_SAMPLE_DESC {
	  UINT Count;
	  UINT Quality;
	};
	*/
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(
			NULL,			//A pointer to the video adapter to use when creating a device. Pass NULL to use the default adapter
			d3dDriverType,	// D3D_DRIVER_TYPE , which represents the driver type to create.
			NULL,			//A handle to a DLL that implements a software rasterizer
			createDeviceFlags,	//The runtime layers to enable 
			&d3dFeatureLevel_required,	//A pointer to an array of D3D_FEATURE_LEVELs, which determine the order of feature levels to attempt to create.
			numFeatureLevels,		//The number of elements in pFeatureLevels.
			D3D11_SDK_VERSION,		//The SDK version; use D3D11_SDK_VERSION.
			&dxgiSwapChainDesc,		//A pointer to a swap chain description
			&gpIDXGISwapChain,		//Returns the address of a pointer to the IDXGISwapChain object that represents the swap chain used for rendering
			&gpID3D11Device,		//Returns the address of a pointer to an ID3D11Device object that represents the device created. If this parameter is NULL, no ID3D11Device will be returned
			&d3dFeatureLevel_acquired, //Returns a pointer to a D3D_FEATURE_LEVEL, which represents the first element in an array of feature levels supported by the device.
			&gpID3D11DeviceContext);  //Returns the address of a pointer to an ID3D11DeviceContext object that represents the device context.
									  //he ID3D11DeviceContext interface represents a device context which generates rendering commands.
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	
	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "D3D11CreateDeviceAndSwapChain() Failed . \n");
		fclose(gpFile);
		return(hr);
		
	}
	else
	{

		fopen_s(&gpFile, gszLogFileName, "a+");
		
		fprintf_s( gpFile,"D3D11CreateDeviceAndSwapChain() Succeeded . \n");
	
		fprintf_s(gpFile, "The Chosen Driver Is of ");
		
		if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
		{
			fprintf_s(gpFile, "Hardware Type. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
		{
			fprintf_s(gpFile, "SoftWareType. \n");
		}
		else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
		{
			fprintf_s(gpFile, "Reference Type. \n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown Type. \n");
		}
		
		fprintf_s(gpFile, "The supported Highest Feature Level is ");
		if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
		{
			fprintf_s(gpFile, "11.0\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
		{
			fprintf_s(gpFile, "10.1\n");
		}
		else if (d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
		{
			fprintf_s(gpFile, "10.0\n");
		}
		else
		{
			fprintf_s(gpFile, "Unknown.\n");
		}


		fclose(gpFile);
	}


	//initialize shader ,  input layouts , constant buffers
	const char *vertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
		"float4x4 worldViewProjectionMatrix;"\
		"}"\
		"float4 main(float4 pos:POSITION):SV_POSITION"\
		"{"\
		"float4 position = mul(worldViewProjectionMatrix,pos);"\
		"return(position);"\
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
				fprintf_s(gpFile, "D3DCompile() Failed for Vertex Shader: %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
				fclose(gpFile);
				pID3DBlob_Error->Release();
				pID3DBlob_Error = NULL;
				return (hr);
			}
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Succeeded for vertex shader.\n");
			fclose(gpFile);
		}
	
		//create a vertex shader from compiled shader
		hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);

		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "CreateVertexShader() Succeeded.\n");
			fclose(gpFile);
		}
	
		//Sets a  shader to the device.
		gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);

		const char* pixelShaderSourceCode =
			"float4 main(void):SV_TARGET"\
			"{"\
			"return  (float4(1.0f,1.0f,1.0f,1.0f));"\
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
				fprintf_s(gpFile, "D3DCompile() Failed for Pixel Shader: %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
				fclose(gpFile);
				pID3DBlob_Error->Release();
				pID3DBlob_Error = NULL;
				return (hr);
			}
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "D3DCompile() Succeeded for pixel shader.\n");
			fclose(gpFile);
		}
		
		hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(), pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);

		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "CreatePixelShader() failed.\n");
			fclose(gpFile);
		}

		//Sets a pixel shader to the device.
		gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
		pID3DBlob_PixelShaderCode->Release();
		pID3DBlob_PixelShaderCode = NULL;

		float vertices[] = 
		{
			0.0f,50.0f,0.0f,
			50.0f,-50.0f,0.0f,
			-50.0f,-50.0f,0.0f
		};
				
		//create vertex buffer
		//keeping the analogy I keep the (VBO)vertex buffer initialization here
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(vertices);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		
		//create vertex buffer
		hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer);
		//check failure
		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Failed for Vertex Buffer\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "ID3D11Device::CreateBuffer() Succeeded for Vertex Buffer.\n");
			fclose(gpFile);
		}

		//copy vertices into above vertex buffer now
		//create a mapped subresource
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//mapping a buffer to synchronize the cpu and gpu read write access
		//first NULL- index offset , second NULL is wait for event finish
		gpID3D11DeviceContext->Map(
			gpID3D11Buffer_VertexBuffer,	//A pointer to a ID3D11Resource interface.
			NULL,							//Index number of the subresource.
			D3D11_MAP_WRITE_DISCARD,		//A D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource.
			NULL,							//Flag that specifies what the CPU does when the GPU is busy. This flag is optional.
			&mappedSubresource);			//A pointer to the D3D11_MAPPED_SUBRESOURCE structure for the mapped subresource.
	
		
		memcpy(mappedSubresource.pData, vertices, sizeof(vertices));
		gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer, NULL);

		
		//create input layout for IA
		D3D11_INPUT_ELEMENT_DESC inputElementDesc;	
		inputElementDesc.SemanticName = "POSITION";				//The HLSL semantic associated with this element in a shader input-signature.
		inputElementDesc.SemanticIndex = 0;						//The semantic index for the element
		inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;	//32 x 3 bit
		inputElementDesc.InputSlot = 0;							//An integer value that identifies the input-assembler (see input slot)
		inputElementDesc.AlignedByteOffset = 0;					//Optional. Offset (in bytes) between each element
		inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//vertex attribute in
		inputElementDesc.InstanceDataStepRate = 0;							//stride
		
		hr = gpID3D11Device->CreateInputLayout(&inputElementDesc, 1, pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), &gpID3D11InputLayout);
			
		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "gpID3D11Device::CreateInputLayout() Failed\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "gpID3D11Device::CreateInputLayout() Succeeded.\n");
			fclose(gpFile);
		}

		gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);
		pID3DBlob_VertexShaderCode->Release();
		pID3DBlob_VertexShaderCode = NULL;

		//define and set the constant buffer
		D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
		ZeroMemory(&bufferDesc_ConstantBuffer,sizeof(D3D11_BUFFER_DESC));
		bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
		bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer);


		if (FAILED(hr))
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() Failed\n");
			fclose(gpFile);
			return(hr);
		}
		else
		{
			fopen_s(&gpFile, gszLogFileName, "a+");
			fprintf_s(gpFile, "gpID3D11Device::CreateBuffer() Succeeded.\n");
			fclose(gpFile);
		}

		gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
		
		//d3d clear color (blue)
		gClearColor[0] = 0.0f;
		gClearColor[1] = 0.0f;
		gClearColor[2] = 1.0f;
		gClearColor[3] = 0.0f;



		gOrthographicProjectionMatrix = XMMatrixIdentity();


	//call resize for first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() failed.\n");
		fclose(gpFile);
		return (hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "resize() succeeded.\n");
		fclose(gpFile);

	}
	return(S_OK);
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

	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,0);
	
	//again get back buffer from swap chain
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);

	//Get render target view from d3d11 device using above back buffer
	//A rendertarget is a resource that can be written by the output - merger stage at the end of a render pass.Each render - target should also have a corresponding depth - stencil view
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);

	if (FAILED(hr))
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() failed.\n");
		fclose(gpFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "ID3D11Device::CreateRenderTargetView() Succeeded.\n");
		fclose(gpFile);
	}

	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;
	
	//set render target view as target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

	//set viewport
	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)width;
	d3dViewPort.Height = (float)height;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;
	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);
	

	//set orthographic matrix
	if (width <= height)
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(-100.0f, 100.0f, -100.0f*((float)height / (float)width), 100.0f*((float)height / (float)width), -100.0f, 100.0f);
	}
	else
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(-100.0f*((float)width / (float)height), 100.0f*((float)width / (float)height), -100.0f, 100.0f,-100.0f,100.0f);

	}

	return(hr);
}


void display(void)
{
	//create render target view for chosen color

	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//select vertex buffer to display
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	//select geometry primitive
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//translation is concerned with world matrix transformation
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	//final world view projection matrix
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gOrthographicProjectionMatrix;

	//load data into constant buffer
	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	//draw vertex buffet to render target
	gpID3D11DeviceContext->Draw(3, 0);

	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);
}


void uninitialize(void)
{
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

	if (gpID3D11Buffer_VertexBuffer)
	{
		gpID3D11Buffer_VertexBuffer->Release();
		gpID3D11Buffer_VertexBuffer = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}

	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
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

	if(gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpFile)
	{
		fopen_s(&gpFile, gszLogFileName, "a+");
		fprintf_s(gpFile, "uninitialize() Succeeded\n");
		fprintf_s(gpFile, "Log file Is successfully Closed.");
		fclose(gpFile);
	}
}




	