#include <Windows.h>
#include <stdio.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include "WICTextureLoader.h"

#pragma warning(disable:4838)
#include "XNAMath/xnamath.h"
#include <comdef.h>


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"DirectXTK.lib")



#define WIN32_LEAN_AND_MEAN
//whats this ?
//A macro : going back to windows 16 bit philosophy of bare minimum set of header files for
//writing a bare bones windows program
#define WIN_WIDTH 1000
#define WIN_HEIGHT 800

FILE* gzFile = NULL; //a file pointer 
char gszLogFileName[] = "log.txt"; //byte array

HWND ghwnd; //a handle
bool gbActiveWindow = false;
bool gbIsEscapeKeyIsPressed = false;
unsigned int gLightKeyIsPressed = 1;

//a file pointer
FILE* gpFile = NULL;

//Direct3D used Interfaces

ID3D11Device* device = NULL;
ID3D11DeviceContext* context = NULL;
ID3D11RenderTargetView* rendertargetview = NULL;

IDXGISwapChain* gpIDXGISwapChain = NULL;

ID3D11VertexShader* gpID3D11VertexShader = NULL;
ID3D11PixelShader* gpID3D11PixelShader = NULL;
ID3D11Buffer* gpID3D11VertexBufferPositionTriangle = NULL;
ID3D11Buffer* gpID3D11VertexBufferColorTriangle = NULL;
ID3D11Buffer* gpID3D11VertexBufferPositionSquare = NULL;
ID3D11Buffer* gpID3D11VertexBufferColorSquare = NULL;
ID3D11Buffer* gpID3D11VertexBufferCubeNormals = NULL;


ID3D11Buffer* gpID3D11ConstantBuffer = NULL;
ID3D11InputLayout* gpID3D11InputLayout = NULL;
ID3D11RasterizerState* gpID3D11RasterizerState = NULL;
ID3D11DepthStencilView* gpID3D11DepthStencilView = NULL;


//Texture parts
ID3D11ShaderResourceView* gpID3D11ShaderResourceView_Texture_Pyramid = NULL;
ID3D11SamplerState* gpID3D11SamplerState_Texture_Pyramid = NULL;
ID3D11ShaderResourceView* gpID3D11ShaderResourceView_Texture_Cube = NULL;
ID3D11SamplerState* gpID3D11SamplerState_Texture_Cube = NULL;


//projection matrix
XMMATRIX gPerspectiveProjectionMatrix;


//the constants required for vertex shader are here
//we prepare for lighting now !
struct CBUFFER
{
	XMMATRIX worldViewMatrix;
	XMMATRIX projectionMatrix;
	XMVECTOR Ld;
	XMVECTOR Kd;
	XMVECTOR Lpos;
	unsigned int toggle;
};



//clear color array
float gClearColor[4];

//rotation angle
float gAngle;



//helper function to print in file

void trace(const char* txt)
{
	fopen_s(&gpFile, gszLogFileName, "a+");
	fprintf_s(gpFile, txt);
	fprintf_s(gpFile,"\n");

	fclose(gpFile);
}


//callback
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//why callback ?
// Because we are registering this function to Windows, which allow us to
// manage screen and keyboard events
//a callback is like a premium event handler which allows us to manage 
//specific events regarding a Window on a screen and keyboard messages to it


//WHAT IS WINMAIN ?
//entry point function to any windows program
//predefined arguments 
//windows executable instruction flow starts from here
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR longpointerstring, int CmdShow)
{
	//ignore the prototypes for flow
	HRESULT initialize(void);
	void display(void);
	void uninitialize(void);
	void update(void);

	//first thing to do in any graphics program is to have a bare
	//minimum window created on screen on which we would show colours

	//lets get our bare minimum native window

	//firstly we need some structures filled in
	//and some variables to be declared

	//some digression here
	//before we begin , lets create a log file to dump our errors

	if (fopen_s(&gpFile, gszLogFileName, "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file cannot be created. exitting ..."),TEXT("error"),MB_OK);
		exit(0);
	}
	else
	{
		fprintf_s(gpFile, "file created \n");
		fclose(gpFile);
	}

	//structure which defines my window attributes
	WNDCLASSEX wndclass;

	TCHAR szAppName[] = TEXT("Direct 3d 11");

	//some handles declared to store 
	HWND hwnd;
	//a boolean to keep our game loop running
	bool bDone = false;

	//message to collect from the system about events logged for window in our class
	MSG msg;

	//lets fill in
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	//redraw entire window if width of client area changed
	//redraw entire window if height of client area changed
	//thirdly every window will get a new unique device context to be defined

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

	//once class attributes are defined 
	//we register our class to the system , where it keeps a map 
	//when we call createwindow func , the system uses a look up table
	//this table stores classname against hInstance and uses it to retrive 
	//window attributes

	RegisterClassEx(&wndclass);


	//Create window
	hwnd = CreateWindow(szAppName, TEXT("direct 3d 11 Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		trace("create window failed");
	}

	ghwnd = hwnd;
	ShowWindow(hwnd,CmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	

	//ok lets go further now
	//our device context won't be able to do animation
	//we need a powerfull context
	//lets create d3d context 
	//we do it in initialize
	HRESULT hr;
	hr = initialize();

	while (bDone == false)
	{
		// check thread message que for posted messages
		//have a look at all the messages dropped in our mailbox
		//these messages are only specific to our window
		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			//fill in all messages in structure
			//for any window of current thread
			//max min filters
			//remove the message from the que after retrieving
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
			//game loop render function here

			if (gbActiveWindow == true)
			{
				if (gbIsEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
			}
			update();
			display();
		}
	}

	uninitialize();
	return ((int)msg.wParam);
}





// my call back defination
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
		//callback to handle all the posted messages

	switch (iMsg)
	{	
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
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
				case 'l':
				case 'L':
					if (gLightKeyIsPressed)
					{
						gLightKeyIsPressed = 0;

					}
					else
					{
						gLightKeyIsPressed = 1;
					}

					break;
				case VK_ESCAPE:
					gbIsEscapeKeyIsPressed = true;
					break;
			}
			break;

		default:
			break;
	}

	//send it to default handler if you don't know what to do with the message
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}





//our functions


HRESULT initialize(void)
{
	//function signatures
	void uninitialize(void);
	HRESULT resize(int, int);
	HRESULT LoadD3DTexture(const wchar_t* textureFileName, ID3D11ShaderResourceView **ppID3DShaderResourceView);

	//variable declaration
	HRESULT hr;
	//baam!
	//A driver type variable..answers are coming...
	D3D_DRIVER_TYPE d3dDriverType;

	//here comes the fun
	//remember I told you that our device context was not strong enough to show animations 
	//It was called DC , which helps GDI (Graphics Device Interface) of windows to do things.
	//things like creating graphics objects and device types
	//device types : it could be display  , bitmap , printer like output methods.
	//The GDI acts as an interface between the application and device driver
	//But GDI is immediate mode graphics , is brief all we do is handle in invalidated client area of window
	//using WM_PAINT and then decide to do stuff. And the stuff we can do is formated texts and some 2D graphics
	//But we are here for animation and game creating potential, so we rather write retained mode programs.
	//retained mode programs allow us to write series of instruction once and the system itself does the WM_PAINT stuff.
	//Although the analogy is correct , WM_PAINT is a derogatory word for the stuff we can do in retained mode.
	//Now in order to enter retained mode we use Direct 3D.
	//D3D gives us better access to the graphics card also called as adapter. 
	//Using D3D we can directly program GPU and draw stuff on it , the GPU then displays it on the screen.


	//Lets begin then:
	//We need Direct3D Interfaces which will have the required methods for us to Access this GPU.
	//These Interfaces are ID3D11Device (virtual adapter) and ID3D11DeviceContext ( the pipeline interface)

	//So how do we get them ? whom to ask ?
	//-> We need to call CreateDevice API to get these both interfaces
	//lets do it

	//The API Prototype is 
	/*RESULT D3D11CreateDevice(
		IDXGIAdapter*			 pAdapter,
		D3D_DRIVER_TYPE          DriverType,
		HMODULE                  Software,		->> using software rasterizer ? then provide its handle
		UINT                     Flags,			->> device creation flags
		const D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT                     FeatureLevels,
		UINT                     SDKVersion,
		ID3D11Device**			 ppDevice,
		D3D_FEATURE_LEVEL*		 pFeatureLevel,
		ID3D11DeviceContext**	 ppImmediateContext
		);
	*/

	//just like we prepared these parameter attributes for createWindow , we have to for createDevice

	//What sort of D3D Driver does our application want ?
	D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE,	// gives us D3D features in hardware, hardware accelarated or software if not. primary driver used by all applications for maximum performance
										 D3D_DRIVER_TYPE_WARP,		// high performance software implementation , it is a custom sofware raterizer used to test your algorithms on a hardware
										 D3D_DRIVER_TYPE_REFERENCE,	// gives us all the latest features, this is software implementation and used for debug and testing purpose
										 D3D_DRIVER_TYPE_SOFTWARE,  // completely software driver, slow !
										 D3D_DRIVER_TYPE_NULL,      // no rendering with this , debuggin non rendering api
										 D3D_DRIVER_TYPE_UNKNOWN	// unknown
	};
	// so we will use D3D_DRIVER_TYPE_HARDWARE .

	//next feature levels of D3D our application wants to run
	//Feature levels are sub versions of versions i.e d9 can have 9_1 9_2 etc.
	D3D_FEATURE_LEVEL d3dFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL d3dFeatureAcquired;



	//now we need a buffer on GPU to draw pixels to
	//these buffers are called swap chain
	//we create a swap chain using CreateSwapChain API
	//first the API craeteswapchain requires attribute parameters

	/*typedef struct DXGI_SWAP_CHAIN_DESC {
		  DXGI_MODE_DESC   BufferDesc;
		  DXGI_SAMPLE_DESC SampleDesc;
		  DXGI_USAGE       BufferUsage;
		  UINT             BufferCount;
		  HWND             OutputWindow;
		  BOOL             Windowed;
		  DXGI_SWAP_EFFECT SwapEffect;
		  UINT             Flags;
										  } DXGI_SWAP_CHAIN_DESC;*/

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	swapChainDesc.BufferDesc.Width = WIN_WIDTH;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = ghwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		d3dFeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&gpIDXGISwapChain,
		&device,
		&d3dFeatureAcquired,
		&context
	);

	if (FAILED(hr))
	{
		trace("swap chain creation failed");
		_com_error err(hr);
		MessageBox(NULL, err.ErrorMessage(), TEXT("error"), MB_OK);

		uninitialize();
		exit(0);
	}
	else
	{
		trace("Device , DeviceContext and SwapChain Created Succesfully");

		if (d3dFeatureAcquired == D3D_FEATURE_LEVEL_11_1)
		{
			trace("feature acquired: 11_1");
		}
		else if (d3dFeatureAcquired == D3D_FEATURE_LEVEL_11_0)
		{
			trace("feature acquired: 11_0");
		}
		else if (d3dFeatureAcquired == D3D_FEATURE_LEVEL_10_1)
		{
			trace("feature acquired: 10_1");
		}
		else if (d3dFeatureAcquired == D3D_FEATURE_LEVEL_10_0)
		{
			trace("feature acquired: 10_0");
		}
		else
		{
			trace("feature level is in 9");
		}
	}


	//finally swap chain created
	//now we have interfaces to access GPU resources and the pipeline and a swap chain to display rendering results, now next lets link it all together.
	//The shader pipeline requires a resource to draw pixels into.
	//We want to create this resource and use it as back buffer for pixel shader to draw into and then read this texture back to swap chain.
	//since the size of this texture should map the swap chain size and since swap chain size could change in resize, we now go do this in resize function.

	//once back from resize continue
	//now we prepare shaders very similar to opengl

	const char* vertexShaderSourceCode =
		"cbuffer ConstantBuffer"\
		"{"\
			"float4x4 worldViewMatrix;"\
			"float4x4 projectionMatrix;"\
			"float4 lightDiffuse;"\
			"float4 materialDiffuse;"\
			"float4 lightPosition;"\
			"uint keyPressed;"\
		"}"\
		"struct vertex_output"\
		"{"\
		"float4 position: SV_POSITION;"\
		"float2 texcoords: TEXCOORD;"\
		"float4 diffuseColor: COLOR;"\
		"};"
		"vertex_output main(float4 pos:POSITION ,float2 texcoords:TEXCOORD, float4 norm:NORMAL)"\
		"{"\
		"vertex_output output;"\
		"output.position = mul(mul(projectionMatrix,worldViewMatrix),pos);"\
		"float4 eyeCoordinates = mul(worldViewMatrix,pos);"\
		"float3 tnorm = normalize(mul( (float3x3)worldViewMatrix,(float3)norm));"\
		"float3 lightDirection = (float3)normalize(lightPosition - eyeCoordinates);"\
		"if(keyPressed == 1)"\
		"{"\
			"float NL = max(dot(tnorm,lightDirection),0.0);"\
			"output.diffuseColor = mul(materialDiffuse,NL);"\
		"}"\
		"else"\
		"{"\
			"output.diffuseColor = float4(0.0,0.0,0.0,0.0);"\
		"}"\
		"output.texcoords = texcoords;"\
		"return(output);"\
		"}";

	ID3DBlob* pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob* pID3DBlob_Error = NULL;

	/*
	  HRESULT D3DCompile(
	  LPCVOID                pSrcData,
	  SIZE_T                 SrcDataSize,
	  LPCSTR                 pSourceName,
	  const D3D_SHADER_MACRO *pDefines,
	  ID3DInclude            *pInclude,
	  LPCSTR                 pEntrypoint,
	  LPCSTR                 pTarget,
	  UINT                   Flags1,
	  UINT                   Flags2,
	  ID3DBlob               **ppCode,
	  ID3DBlob               **ppErrorMsgs
	);
	*/

	hr = D3DCompile(vertexShaderSourceCode, lstrlenA(vertexShaderSourceCode) + 1, "VS", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &pID3DBlob_VertexShaderCode, &pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			trace("D3DCompile failed for vertex shader");
			trace((const char*)pID3DBlob_Error->GetBufferPointer());
			pID3DBlob_Error->Release();
			pID3DBlob_Error = nullptr;
			return hr;
		}
	}
	else
	{
		trace("vertex shader compiled successfully");
	}

	hr = device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(), pID3DBlob_VertexShaderCode->GetBufferSize(), NULL, &gpID3D11VertexShader);

	if (FAILED(hr))
	{
		trace("vertex shader creation failed");

	}
	else
	{
		trace("vertex shader created successfully");
	}

	//tell the ID3D11DeviceContext to set vertex shader in pipeline
	context->VSSetShader(gpID3D11VertexShader, 0, 0);

	
	//now onto pixel shader
	const char* pixelShaderSourceCode =
		"Texture2D myTexture2d;"\
		"SamplerState mySamplerState;"\
		"float4 main(float4 position:SV_POSITION,float2 texcoords:TEXCOORD, float4 diffuseColor: COLOR):SV_TARGET"\
		"{"\
		"float4  fTexture = myTexture2d.Sample(mySamplerState,texcoords);"\
		"float4 fColor = diffuseColor;"\
		"return ((fColor+fTexture));"\
		"}";


	ID3DBlob* pID3DBlob_PixelShaderCode = nullptr;
	pID3DBlob_Error = nullptr;

	hr = D3DCompile(pixelShaderSourceCode, lstrlenA(pixelShaderSourceCode) + 1, "PS", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pID3DBlob_PixelShaderCode, &pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != nullptr)
		{
			trace("D3DCompile failed for pixel shader:");
			trace((const char*)pID3DBlob_Error->GetBufferPointer());
			pID3DBlob_Error->Release();
			pID3DBlob_Error = nullptr;
			return (hr);
		}
	}
	else
	{
		trace("pixel shader compiled success");
	}


	hr = device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(), pID3DBlob_PixelShaderCode->GetBufferSize(), NULL, &gpID3D11PixelShader);

	if (FAILED(hr))
	{
		trace("pixel shader creation failed");
	}
	else
	{
		trace("pixel created successfully");
	}

	context->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = nullptr;

	//now on send in geometry to be drawn
	//mapping data on to GPU buffer to push down further in pipeline

	float positions_triangle[] = { 0.0f,1.0f,0.0f,1.0,-1.0f,0.0f,-1.0f,-1.0f,0.0f };
	float colors_triangles[] = { 1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f };

	float positions_square[] =
	{
		1.0f,1.0f,0.0f,
		1.0f,-1.0f,0.0f,
	   -1.0f,-1.0f,0.0f,
	   -1.0f,-1.0f,0.0f,
	   -1.0f, 1.0f,0.0f,
	    1.0f, 1.0f,0.0f
	};

	float colors_square[] =
	{
		0.57f,0.79f,0.92f,
		0.57f,0.79f,0.92f,
		0.57f,0.79f,0.92f,
		0.57f,0.79f,0.92f,
		0.57f,0.79f,0.92f,
		0.57f,0.79f,0.92f
	};



	float pyramid_vertices[] =
	{
		//front
		 0.0f,1.0f,0.0f,
		-1.0f,-1.0f,1.0f,
		 1.0f,-1.0f, 1.0f,

		 //right
		 0.0f,1.0f,0.0f,
		 1.0f,-1.0f,1.0f,
		 1.0f,-1.0f,-1.0f,

		 //back 
		 0.0f,1.0f,0.0f,
		 1.0f,-1.0f,-1.0f,
		 -1.0f,-1.0f,-1.0f,

		 //left
		 0.0f,1.0f,0.0f,
		 -1.0f,-1.0f,-1.0f,
		 -1.0f,-1.0f, 1.0f
	};


	float pyramid_color[] =
	{
		//triangle of front side
		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f,

		//triangle of right side
		1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,

		//triangle of back side
		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f,

		//triangle of left side
		1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f
	};

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

	float pyramid_texcoords[] =
	{
		//front side
		0.5f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		//right side
		0.5f,1.0f,
		1.0f,0.0f,
		0.0f,0.0f,

		//back side
		0.5f,1.0f,
		1.0f,0.0f,
		0.0f,0.0f,

		//left side
		0.5f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f
	};




	float cube_colors[] =
	{
		//top
		//triangle 1
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

		//triangle 2
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

		//bottom
		//triangle 1
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		//triangle 2
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		//front 
		//triangle 1
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,

		//triangle 2
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,

		//back 
		//triangle 1
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		//triangle 2
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,
		0.0f,1.0f,1.0f,

		//left
		//triangle 1
		1.0f,0.0f,1.0f,
		1.0f,0.0f,1.0f,
		1.0f,0.0f,1.0f,

		//triangle 2
		1.0f,0.0f,1.0f,
		1.0f,0.0f,1.0f,
		1.0f,0.0f,1.0f,

		//right
		//triangle 1
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,

		//triangle 2
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,
		1.0f,1.0f,0.0f,
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
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,0.0f,
		1.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,

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


	float cube_normals[] =
	{
		//top
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,1.0f,0.0f,

		//bottom
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,

		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		0.0f,-1.0f,0.0f,

		//front
		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,

		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,
		0.0f,0.0f,-1.0f,

		//back
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,

		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,
		0.0f,0.0f,1.0f,

		//left
		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,

		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,
		-1.0f,0.0f,0.0f,

		//right
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,

		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
	};




	//create the vertex buffer
	//like we created VBO in opengl
	//the question which immediately arises is who is going to use this resource buffer ?
	//-> The answer might start from the fact that who is going to use this buffer indeed.
	//Its the GPU which is going to run the pipe line , hence at various stages , GPU will require the data in retained form on its memory.
	//This is exactly what retained mode is and we want to pass sequential instructions once from the application and then the D3D system will take care of updating it frame by frame.


	//create buffer resource----------------------------------------------------------------------------------------------
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;			//vertex buffer type
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(pyramid_vertices); //size
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;						//read from GPU write from CPU
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			//cpu has write permissions on the buffer resource

	//create vertex buffer now
	hr = device->CreateBuffer(
		&bufferDesc,			//buffer description
		NULL,					//no initial data , the data will be mapped later as a pointer to D3D11SUBRESOURCE
		&gpID3D11VertexBufferPositionTriangle	//created buffer
		);

	if (FAILED(hr))
	{
		trace("vertex buffer creation FAILED");
	}
	else
	{
		trace("vertex buffer created SUCCESS ");
	}
	//-------------------------------------------------------------------------------------------------------------------------
	//create the subresource to be mapped onto buffer	
	//subresource because it is on CPU (about to be mapped on GPU) and resource is on GPU buffer.
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//use ID3D11DeviceContext to map the vertices data as the subresource
	context->Map(
		gpID3D11VertexBufferPositionTriangle,		//buffer on which to map
		NULL,						//index of subresource
		D3D11_MAP_WRITE_DISCARD,	//how the resource is accessed by cpu, write only type by discarding the previous data on the buffer
		NULL,						//what should cpu do when gpu is busy , we keep it default
		&mappedSubresource			//our subresource to be mapped onto the main resource i.e vertexbuffer on gpu
		);
	memcpy(mappedSubresource.pData, pyramid_vertices, sizeof(pyramid_vertices));
	//once copied just unmap
	context->Unmap(gpID3D11VertexBufferPositionTriangle, NULL);

	//question arises here that , why not provide pInitial data while creating the buffer itself, why map unmap later using context ??????????????????????????


	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(pyramid_texcoords);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&bufferDesc, NULL, &gpID3D11VertexBufferColorTriangle);

	if (FAILED(hr))
	{
		trace("color buffer created FAILED");
	}
	else
	{
		trace("color buffer created SUCCESS");
	}

	//map the color subresource
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(gpID3D11VertexBufferColorTriangle, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, pyramid_texcoords, sizeof(pyramid_texcoords));
	context->Unmap(gpID3D11VertexBufferColorTriangle, NULL);


	//-------------------------------------SQUARE or Cube ----------------------------------------------------------------------------------
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float)* _ARRAYSIZE(cube_vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	hr = device->CreateBuffer(&bufferDesc, NULL, &gpID3D11VertexBufferPositionSquare);
	
	if (FAILED(hr))
	{
		trace("position buffer for square created FAILED");
		return hr;
	}
	else
	{
		trace("position buffer for square created SUCCESS");
	}


	//map subresource for square position
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(gpID3D11VertexBufferPositionSquare, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cube_vertices, sizeof(cube_vertices));
	context->Unmap(gpID3D11VertexBufferPositionSquare, NULL);


	//square colors buffer
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float)* _ARRAYSIZE(cube_texcoords);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&bufferDesc, NULL, &gpID3D11VertexBufferColorSquare);

	if (FAILED(hr))
	{
		trace("color buffer for square created FAILED");
		return hr;
	}
	else
	{
		trace("color buffer for square created SUCCESS");
	}


	//map subresource
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(gpID3D11VertexBufferColorSquare, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, cube_texcoords, sizeof(cube_texcoords));
	context->Unmap(gpID3D11VertexBufferColorSquare, NULL);


	//normals for the cube
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(cube_normals);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	hr = device->CreateBuffer(&bufferDesc, NULL, &gpID3D11VertexBufferCubeNormals);

	if (FAILED(hr))
	{
		trace("Normal buffer for Cube Creation failed");
	}
	else
	{
		trace("Normal buffer for Cube Created  succesfully");
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	context->Map(gpID3D11VertexBufferCubeNormals, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, cube_normals, sizeof(cube_normals));
	context->Unmap(gpID3D11VertexBufferCubeNormals,NULL);

	//------------------------------------------------------------------------------------------------------------------------------------

	//now that the vertices have reached GPU resource buffer as vertex buffer
	//we must tell GPU how to read those vertices
	//just like how we did in vertexattribpointer in the OPENGL
	//D3D11 gives us a createInputLayout API , which we must use 

	//https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-input-assembler-stage-getting-started
	//createInputLayout needs inputElementDescriptor
	// In comes the Input element descriptor which will do exactly that

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[3];

	inputElementDesc[0].SemanticName = "POSITION"; //attrib pointer remember ?
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // just like 3 * float
	inputElementDesc[0].InputSlot = 0;
	inputElementDesc[0].AlignedByteOffset = 0; //offset in buffer to access the first element
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;  // what is per instance data ????
	inputElementDesc[0].InstanceDataStepRate = 0; //the hell is this ?

	inputElementDesc[1].SemanticName = "TEXCOORD"; //attrib pointer remember ?
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT; // just like 3 * float
	inputElementDesc[1].InputSlot = 1;
	inputElementDesc[1].AlignedByteOffset = 0; //offset in buffer to access the first element
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;  // what is per instance data ????
	inputElementDesc[1].InstanceDataStepRate = 0; //the hell is this ?

	inputElementDesc[2].SemanticName = "NORMAL";
	inputElementDesc[2].SemanticIndex = 0;
	inputElementDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[2].InputSlot = 2;
	inputElementDesc[2].AlignedByteOffset = 0;
	inputElementDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[2].InstanceDataStepRate = 0;


	hr = device->CreateInputLayout(
		inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout
	);

	if (FAILED(hr))
	{
		trace("createInputLayout() failed");
		return hr;
	}
	else
	{
		trace("createInputLayout() success");
	}


	//now that we have created input layout , lets plug it in the pipeline
	context->IASetInputLayout(gpID3D11InputLayout);
	//release the vertex shader byte code we have held
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = nullptr;
	
	//hooray!
	// data is plugged in to the pipeline , IA knows how and where to get the data on GPU memory 
	//now onto next part

	//we also need to send uniforms to the shaders along with vertex data right
	//uniforms are sent using constant buffers
	//just like we created vertex buffer , lets create constant buffer

	//create a D3D11 buffer description

	D3D11_BUFFER_DESC constantBuffer;
	ZeroMemory(&constantBuffer, sizeof(D3D11_BUFFER_DESC));
	
	constantBuffer.Usage = D3D11_USAGE_DEFAULT;	//GPU read GPU write
	constantBuffer.ByteWidth = sizeof(CBUFFER);
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//tell that this is a constant buffer for uniforms 


	hr = device->CreateBuffer(&constantBuffer, NULL, &gpID3D11ConstantBuffer);
	//check
	if (FAILED(hr))
	{
		trace("failed to create constant buffer");
	}
	else
	{
		trace("constant buffer created succesfully");
	}

	//now we have another buffer resource on GPU i.e constant buffer , this constant buffer is to be used by the vertex stage ,so lets plug it in
	context->VSSetConstantBuffers(0, 1, &gpID3D11ConstantBuffer);
	//done


	//what is this ??
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	//tweaking the accessible attributes of the rasterizer according to need
	//since in animation we need backface culling of for now , lets just tell the rasterizer to do that

	/*
	typedef struct D3D11_RASTERIZER_DESC {
		  D3D11_FILL_MODE FillMode;
		  D3D11_CULL_MODE CullMode;
		  BOOL            FrontCounterClockwise;
		  INT             DepthBias;
		  FLOAT           DepthBiasClamp;
		  FLOAT           SlopeScaledDepthBias;
		  BOOL            DepthClipEnable;
		  BOOL            ScissorEnable;
		  BOOL            MultisampleEnable;
		  BOOL            AntialiasedLineEnable;
	} D3D11_RASTERIZER_DESC;
	
	*/

	//fill in the descriptor first
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	hr = device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);

	if (FAILED(hr))
	{
		trace("CreateRasterizerState has failed");
	}
	else
	{
		trace("CreateRasterizerState success");
	}

	context->RSSetState(gpID3D11RasterizerState);


	//Here we go for textures 
	hr = LoadD3DTexture(L"Stone.bmp", &gpID3D11ShaderResourceView_Texture_Pyramid);
	
	if (FAILED(hr))
	{
		trace("load3d texture for stone.bmp failed");
	}
	else
	{
		trace("load3d texture for stone.bmp success");
	}

	//sampler for the texture
	//how you sample the texture in the shader 
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Pyramid);

	if (FAILED(hr))
	{
		trace("create sampler state for pyramid failed");
	}
	else
	{
		trace("create sampler state for pyramid is success");
	}
	
	//so you resource view and sampler state are ready for pyramid 
	//now do the same for cube

	//create resource view for cube

	hr = LoadD3DTexture(L"Vijay_Kundali.bmp", &gpID3D11ShaderResourceView_Texture_Cube);

	if (FAILED(hr))
	{
		trace("load3dTexture for vijay kundali texture failed");
	}
	else
	{
		trace("load3dTexture for vijay kundali texture success");
	}

	//create sampler state
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	hr = device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Texture_Cube);

	if (FAILED(hr))
	{
		trace("create sampler state for cube failed");
	}
	else
	{
		trace("create sampler state for cube success");
	}

	

	//pipeline is ready to draw stuff

	//clear color
	gClearColor[0] = 0.0f;
	gClearColor[1] = 0.0f;
	gClearColor[2] = 0.0f;
	gClearColor[3] = 0.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();


	//warm up resize to intialize our back buffer accordingly and create our render target view

	hr = resize(WIN_WIDTH, WIN_HEIGHT);

	if (FAILED(hr))
	{
		trace("warm up resize has failed");
	}
	else
	{
		trace("warm up resize done succesfully");
	}

	//over to drawing now , go to display function
	return S_OK;

}

HRESULT LoadD3DTexture(const wchar_t* textureFileName, ID3D11ShaderResourceView** ppID3DShaderResourceView)
{
	HRESULT hr;

	hr = DirectX::CreateWICTextureFromFile(device, context, textureFileName, 0, ppID3DShaderResourceView);

	if (FAILED(hr))
	{
		trace("create wic texture failed");
	}
	else
	{
		trace("create wid texture success");
	}

	return hr;
}

HRESULT resize(int width, int height)
{
	HRESULT hr = S_OK;

	//free any size dependent resources since we are to recreate them again
	if (rendertargetview)
	{
		rendertargetview->Release();
		rendertargetview = nullptr;
	}

	//IDXGISwapChain should resize its buffers 
	gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	//get the back buffer access from swapchain
	ID3D11Texture2D* pTexture2DBackBuffer;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pTexture2DBackBuffer);

	hr = device->CreateRenderTargetView(pTexture2DBackBuffer, NULL, &rendertargetview);

	if (FAILED(hr))
	{
		trace("rendertargetview creation failed");
	}
	else
	{
		trace("rendertargetview created");
	}

	pTexture2DBackBuffer->Release();
	pTexture2DBackBuffer = NULL;

	//for 3D animation
	//we need depth buffer for depth testing
	//lets do it

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1; 
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* pID3D11Texture2DDepthBuffer;
	device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2DDepthBuffer);

	//having a buffer is not enough , we need the view to access the resource
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//what is this dimension ?
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hr = device->CreateDepthStencilView(pID3D11Texture2DDepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);

	if (FAILED(hr))
	{
		trace("create depth stencil view failed");
	}
	else
	{
		trace("create depth stencil view succesfull");
	}

	pID3D11Texture2DDepthBuffer->Release();
	pID3D11Texture2DDepthBuffer = NULL;


	//set the output target in pipeline using our ID3D11DeviceContext
	context->OMSetRenderTargets(1, &rendertargetview, gpID3D11DepthStencilView);



	//create viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.Width = width;
	viewport.Height = height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	//set the viewport for rendering in raster stage using ID3D11DeviceContext
	context->RSSetViewports(1, &viewport);


	if (width >= height)
	{
		gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	}
	else
	{
		gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), (float)height / (float)width, 0.1f, 100.0f);
	}

	return hr;
}


void update(void)
{
	gAngle += 0.001f;
}

//drawing stuff now
void display(void)
{
	//clear the buffer like opengl right
	//but here instead 
	//we have access to the part of back buffer on which we are going to render
	//this is the render target view
	//we request he context to clear the render target view
	context->ClearRenderTargetView(rendertargetview, gClearColor);
	context->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//plug in the vertex buffer we created earlier to the pipeline
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, &gpID3D11VertexBufferPositionTriangle, &stride, &offset);
	//stride = sizeof(float) * 2;
	//offset = 0;
	//context->IASetVertexBuffers(1, 1, &gpID3D11VertexBufferColorTriangle, &stride, &offset);

	//bind texture and sampler as pixel shader resource
	//context->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture_Pyramid);
	//context->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Pyramid);


	//set the geometry to be assembled from the vertex buffer data to IA
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //GL_TRIANGLES
	//translation is concerned with world matrix transformation
	//we create the worldViewMatrix and send it to the constant buffer resource on GPU
	
	/*
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixRotationY(-gAngle);
	XMMATRIX translationMatrix = XMMatrixTranslation(-2.0f, 0.0f, 9.0f);
	*/


	//transform
	//load data into constant buffer
	//CBUFFER constBuff;
	

	//write into the GPU resource of constant buffer from CPU
	//context->UpdateSubresource(gpID3D11ConstantBuffer, 0, NULL, &constBuff, 0, 0);
	//what is D3D11_BOX , rowpitch , depthpitch ?????
	//drawing finally
	//context->Draw(12, 0);
	
	
	//plug in the vertex buffer we created earlier to the pipeline
	stride = sizeof(float) * 3;
	offset = 0;
	context->IASetVertexBuffers(0, 1, &gpID3D11VertexBufferPositionSquare, &stride, &offset);
	
	stride = sizeof(float) * 2;
	offset = 0;
	context->IASetVertexBuffers(1, 1, &gpID3D11VertexBufferColorSquare, &stride, &offset);


	stride = sizeof(float) * 3;
	offset = 0;
	context->IASetVertexBuffers(2, 1, &gpID3D11VertexBufferCubeNormals, &stride, &offset);


	context->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Texture_Cube);
	context->PSSetSamplers(0, 1, &gpID3D11SamplerState_Texture_Cube);

	//set the geometry to be assembled from the vertex buffer data to IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //GL_TRIANGLES
	//translation is concerned with world matrix transformation
	//we create the worldViewMatrix and send it to the constant buffer resource on GPU
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixRotationX(gAngle);
			 rotationMatrix *= XMMatrixRotationY(gAngle);
			 rotationMatrix *= XMMatrixRotationZ(gAngle);

    XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 10.0f);
		     worldMatrix = rotationMatrix * translationMatrix;

	XMMATRIX wvMatrix = worldMatrix * viewMatrix;

	//load data into constant buffer
	CBUFFER constBuff;
	constBuff.worldViewMatrix = wvMatrix;
	constBuff.projectionMatrix = gPerspectiveProjectionMatrix;
	constBuff.Kd = XMVectorSet(0.5, 0.5f, 0.5f, 1.0f);
	constBuff.Ld = XMVectorSet(0.25f, 0.24f, 0.25f, 1.0f);
	constBuff.Lpos = XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);
	constBuff.toggle = gLightKeyIsPressed;

	//write into the GPU resource of constant buffer from CPU
	context->UpdateSubresource(gpID3D11ConstantBuffer, 0, NULL, &constBuff, 0, 0);
	//what is D3D11_BOX , rowpitch , depthpitch ?????
	//drawing finally
	context->Draw(36, 0);

	//switch buffers
	gpIDXGISwapChain->Present(0, 0);
	//asynch presentation,default presentation

}



void uninitialize(void)
{
	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = nullptr;
	}


	if (gpID3D11VertexBufferPositionSquare)
	{
		gpID3D11VertexBufferPositionSquare->Release();
		gpID3D11VertexBufferPositionSquare = nullptr;
	}

	if (gpID3D11VertexBufferColorSquare)
	{							 
		gpID3D11VertexBufferColorSquare->Release();
		gpID3D11VertexBufferColorSquare = nullptr;
	}

	if (gpID3D11VertexBufferPositionTriangle)
	{
		gpID3D11VertexBufferPositionTriangle->Release();
		gpID3D11VertexBufferPositionTriangle = nullptr;
	}

	if (gpID3D11VertexBufferColorTriangle)
	{
		gpID3D11VertexBufferColorTriangle->Release();
		gpID3D11VertexBufferColorTriangle = nullptr;
	}



	if (gpID3D11ConstantBuffer)
	{
		gpID3D11ConstantBuffer->Release();
		gpID3D11ConstantBuffer = nullptr;
	}

	if (rendertargetview)
	{
		rendertargetview->Release();
		rendertargetview = nullptr;
	}



	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = nullptr;
	}

	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (context)
	{
		context->Release();
		context = nullptr;
	}

	if (gpFile)
	{
		fclose(gpFile);
	}

}







