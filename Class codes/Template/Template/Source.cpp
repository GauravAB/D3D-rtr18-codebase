#include <Windows.h>
#include <stdio.h>

#include <d3d11.h>

#pragma comment(lib,"d3d11.lib")
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


	//d3d clear color (blue)
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 1.0f;
	gClearColor[3] = 0.0f;

	//call resize for first time
	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
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

	//again get render target view from d3d11 device using above back buffer
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
	
	return(hr);
}


void display(void)
{
	//code
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

	//switch between front and back buffers
	gpIDXGISwapChain->Present(0, 0);
}


void uninitialize(void)
{
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




