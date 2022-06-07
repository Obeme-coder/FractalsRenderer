#pragma once
#include "mode.h"

class MenuMode : public Mode {
public:
	MenuMode();
	~MenuMode();
	void MessageLoop();
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Render();
};