#pragma once
#include "mode.h"

class SierpinskiTriangleMode : public Mode {
private:
	PROGRAM color_program;
	VBO triangle_vbo;
	VAO triangle_vao;
	FBO msaa_compute_framebuffer, compute_framebuffer, color_framebuffer;
	TEXTURE msaa_compute_texture, compute_texture, color_texture;

	float colors[3][3] { {1, 1, 0}, {1, 0, 1}, {0, 1, 1}, };
	unsigned int colors_uniform_location[3];

	bool recompute_flag = false;
	bool saved_flag = false;
	bool draw_big_triangle_flag = true;

public:
	SierpinskiTriangleMode();
	~SierpinskiTriangleMode();
	void MessageLoop();
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ResetRecursion();
	void RecursionStep();
	void Color();
	void Render();
};