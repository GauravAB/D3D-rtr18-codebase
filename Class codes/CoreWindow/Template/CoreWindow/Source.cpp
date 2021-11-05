#include <Windows.h>


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

BOOL gbFlag = FALSE;

//signature to callback function 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine , int nCmdShow)
{
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("CoreWindowApplication");

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	RegisterClassEx(&wndclass);
	hwnd = CreateWindow(szAppName, TEXT("CoreWindow"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WIN_WIDTH, WIN_HEIGHT, 0, 0, hInstance,0);
	if (!hwnd)
	{
		MessageBox(NULL, TEXT("failed to create window"), TEXT("error"), MB_OK);
		exit(0);
	}
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (gbFlag == FALSE)
	{
		if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
		{
			if (msg.wParam == WM_QUIT)
			{
			//	MessageBox(NULL, TEXT("WM_QUIT"), TEXT("message received"), MB_OK);
				gbFlag = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			//do rendering stuff
		}
	}

	return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT uIMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uIMsg)
	{
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_ESCAPE:
					DestroyWindow(hwnd);
					gbFlag = TRUE;
					break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			UnregisterClass(TEXT("CoreWindowApplication"), (HINSTANCE)GetModuleHandle(NULL));
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
	}

	//pass to the default callback to check default notification handles
	return DefWindowProc(hwnd, uIMsg, wParam, lParam);
}