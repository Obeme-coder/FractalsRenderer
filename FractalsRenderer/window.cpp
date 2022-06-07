#include "window.h"
#include "main.h"
#include "opengl.h"
#include "modes.h"

HDC dc;
HGLRC rc;
HWND window;
int window_width, window_height;
unique_ptr<Mode> mode;
UINT exit_message_loop_wm;

void InitConsole() {
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
};

int InitWindowClass(const wchar_t* name, HINSTANCE hInstance) {
	WNDCLASSEXW wc = {};

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = name;

	return RegisterClassExW(&wc);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		window = hwnd;

		exit_message_loop_wm = RegisterWindowMessageW(L"exit message");

		RenderInit();

		// ImGui init
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(window);

		int version[2];
		glGetIntegerv(GL_MAJOR_VERSION, &version[0]);
		glGetIntegerv(GL_MINOR_VERSION, &version[1]);
		string version_string = "# version " + to_string(version[0]) + to_string(version[1]) + "0";
		ImGui_ImplOpenGL3_Init(version_string.c_str());

		unsigned char low = 40, high = 255; COLOR colors[3]{ {high, low, low}, {low, high, low}, {low, low, high} };

		mode = unique_ptr<Mode>((Mode*)new MenuMode());
		//mode = unique_ptr<Mode>((Mode*)new SierpinskiTriangleMode());
		//mode = unique_ptr<Mode>((Mode*)new MandelbrotMode());

	} break;
	case WM_CLOSE: {
		DestroyWindow(hwnd);
	} break;
	case WM_DESTROY: {
		ReleaseDC(window, dc);
		wglDeleteContext(rc);
		
		exit(0);
	} break;
	default: {
		if (mode) {
			return mode->WindowProc(hwnd, msg, wParam, lParam);
		}
	} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void GLInit() {
	PIXELFORMATDESCRIPTOR pfd = {
			  sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
			  1,                     // version number  
			  PFD_DRAW_TO_WINDOW |   // support window  
				  PFD_SUPPORT_OPENGL |   // support OpenGL  
				  PFD_DOUBLEBUFFER,      // double buffered  
			  PFD_TYPE_RGBA,         // RGBA type  
			  24,                    // 24-bit color depth  
			  0, 0, 0, 0, 0, 0,      // color bits ignored  
			  0,                     // no alpha buffer  
			  0,                     // shift bit ignored  
			  0,                     // no accumulation buffer  
			  0, 0, 0, 0,            // accum bits ignored  
			  32,                    // 32-bit z-buffer      
			  0,                     // no stencil buffer  
			  0,                     // no auxiliary buffer  
			  PFD_MAIN_PLANE,        // main layer  
			  0,                     // reserved  
			  0, 0, 0                // layer masks ignored  
	};
	dc = GetDC(window);
	int pf = ChoosePixelFormat(dc, &pfd);
	SetPixelFormat(dc, pf, &pfd);

	// create temp rc to load wglCreateContextAttribsARB
	rc = wglCreateContext(dc);
	wglMakeCurrent(dc, rc);
	gladLoadGL();

	int attribs[] { WGL_CONTEXT_MAJOR_VERSION_ARB , 4, WGL_CONTEXT_MINOR_VERSION_ARB , 4, 0};

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	wglDeleteContext(rc);

	rc = wglCreateContextAttribsARB(dc, 0, NULL);
	wglMakeCurrent(dc, rc);

	glViewport(0, 0, window_width, window_height);

	cout << "OpenGL context initialized" << endl;
}