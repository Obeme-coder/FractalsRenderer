#include "main.h"
#include "window.h"
#include "opengl.h"
#include "SierpinskiTriangleMode.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const wchar_t WCLASS_NAME[] = L"gfaod";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	InitConsole();

	ATOM wclass = InitWindowClass(WCLASS_NAME, hInstance);

	window_width = window_height = 900;

	RECT rect{ 0, 0, window_width, window_height};
	AdjustWindowRect(&rect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
	CreateWindowExW(0, WCLASS_NAME, L"title", WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInstance, 0);

	ShowWindow(window, SW_SHOW);

	// main message loop
	while (mode) {
		mode->MessageLoop();
	}
}