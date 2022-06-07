#pragma once
#include "mode.h"

class MandelbrotMode : public Mode {
private:
	PROGRAM mandelbrot_program, mandelbrot_color_program;
	FBO msaa_compute_framebuffer, color_framebuffer;
	VAO screen_vao;
	VBO screen_vbo;
	TEXTURE framebuffer_pos_texture, framebuffer_compute_texture, framebuffer_color_texture;
	TEXTURE gradient_texture;

	// uniforms
	int max_iteration_uniform_location;
	float max_iterations = 35;
	const float max_max_iterations = 2000;
	const float min_max_iterations = 1;
	int power_uniform_location;
	float power = 2;
	int cz_uniform_location;
	glm::vec2 cz{ 1, 1 };
	int c_uniform_location;
	glm::vec2 c{ };
	int zc_uniform_location;
	int pos_uniform_location;
	glm::vec2 pos = { 0, 0 };
	int zoom_uniform_location;
	float zoom = 1;

	int res_uniform_location;
	int grid_density_uniform_location;
	float grid_density = 0.1f;
	int grid_thickness_uniform_location;
	float grid_thickness = 1;
	int grid_color_uniform_location;
	float grid_color[3]{ 196.0 / 255, 37.0 / 255, 37.0 / 255 };
	int draw_grid_uniform_location;
	bool draw_grid = false;

	const float min_zoom = 0.1;
	const float max_zoom = 4500;

	bool recompute_flag = false;
	bool recolor_flag = false;
	bool saved_flag = false;

	bool select_box_is_active = false;
	glm::vec2 select_box_origin;
	float select_box_size;
	const float min_select_box_size = 0.07;
public:
	MandelbrotMode();
	~MandelbrotMode();
	void MessageLoop();
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ComputeMandelbrot();
	void ColorMandelbrot();
	void Render();
	void CalculateSelectBox(LPARAM lParam);
	string GenerateFilename();
	void SaveGif(); // remove later;
};