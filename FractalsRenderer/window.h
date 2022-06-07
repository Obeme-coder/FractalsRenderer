#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <vector>
#include <string>
#include <fstream>
#include "glad.h"
#include "wglext.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"

using namespace std;

class Mode;

extern HDC dc;
extern HGLRC rc;
extern HWND window;
extern int window_width, window_height;
extern unique_ptr<Mode> mode;
extern UINT exit_message_loop_wm;

void InitConsole();
int InitWindowClass(const wchar_t* name, HINSTANCE hInstance);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RenderInit();
void GLInit();