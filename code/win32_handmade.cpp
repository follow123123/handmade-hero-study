#include <windows.h>
 
WNDPROC Wndproc;

LRESULT MainWindowsCallback(
	HWND Window,
	UINT Message,
	WPARAM wParam,
	LPARAM lParam
)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		} break;

		case WM_DESTROY:
		{
			OutputDebugStringA("WM_DESTORY\n");
		} break;

		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

#if 1
		case WM_PAINT:
		{
			OutputDebugStringA("WM_PAINT\n");
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			static DWORD Operation = WHITENESS;
			if (Operation == WHITENESS)
			{
				Operation = BLACKNESS;
			} 
			else 
			{
				Operation = WHITENESS;
			}
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			EndPaint(Window, &Paint);
		}
#endif
		default:
		{
			//OutputDebugStringA("default\n");
			Result = DefWindowProc(Window, Message, wParam, lParam);
		} break;
	}

	return Result;
}

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR     CommandLine,
	int       ShowCode)
{
	WNDCLASS WindowClass = {};

	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowsCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass))
	{
		 HWND WindowHandle = 
			 CreateWindowExA(
				 0,
				 WindowClass.lpszClassName,
				 "Handmade Hero",
				 WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 0,
				 0,
				 Instance,
				 0
			 );
		 if (WindowHandle)
		 {
			 for (;;)
			 {
				 MSG Message;
				 BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				 if (MessageResult > 0)
				 {
					 TranslateMessage(&Message);  
					 DispatchMessage(&Message);
				 } 
				 else
				 {
					 break;
				 }
			 }	
		 } 
		 else
		 {
			 // TODO: logging
		 }
	} 
	else 
	{
		// TODO: Logging
	}

	return 0;
}

