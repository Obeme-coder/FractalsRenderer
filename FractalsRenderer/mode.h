#pragma once
#include <iostream>
#include "windows.h"
#include "glad.h"
#include "khrplatform.h"
#include "opengl.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

class Mode {
public:
	virtual LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void MessageLoop() = 0;
};