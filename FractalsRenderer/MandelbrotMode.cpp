#include "modes.h"
#include "window.h"

MandelbrotMode::MandelbrotMode() {
	// init programs & uniforms
	mandelbrot_program = CreateProgram(L"mandelbrot.vert", L"mandelbrot.frag");
	mandelbrot_color_program = CreateProgram(L"mandelbrot_color.vert", L"mandelbrot_color.frag");

	max_iteration_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "max_iterations");
	power_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "power");
	c_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "c");
	cz_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "cz");
	pos_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "pos");
	zoom_uniform_location = glad_glGetUniformLocation(mandelbrot_program, "zoom");

	res_uniform_location = glad_glGetUniformLocation(mandelbrot_color_program, "res");
	grid_density_uniform_location = glad_glGetUniformLocation(mandelbrot_color_program, "grid_density");
	grid_thickness_uniform_location = glad_glGetUniformLocation(mandelbrot_color_program, "grid_thickness");
	grid_color_uniform_location = glad_glGetUniformLocation(mandelbrot_color_program, "grid_color");
	draw_grid_uniform_location = glad_glGetUniformLocation(mandelbrot_color_program, "draw_grid");

	// create msaa_compute_framebuffer
	glGenFramebuffers(1, &msaa_compute_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, msaa_compute_framebuffer);

	{
		glGenTextures(1, &framebuffer_pos_texture);
		glBindTexture(GL_TEXTURE_2D, framebuffer_pos_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, window_width, window_height, 0, GL_RGB, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer_pos_texture, 0);
	}

	{
		glGenTextures(1, &framebuffer_compute_texture);
		glBindTexture(GL_TEXTURE_2D, framebuffer_compute_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, window_width, window_height, 0, GL_RED, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, framebuffer_compute_texture, 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "fbo error" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create color_framebuffer
	glGenFramebuffers(1, &color_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

	{
		glGenTextures(1, &framebuffer_color_texture);
		glBindTexture(GL_TEXTURE_2D, framebuffer_color_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, framebuffer_color_texture, 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "fbo error" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// load gradient_texture image
	int w, h, channels;

	unsigned char* tex_data = stbi_load("gradient.png", &w, &h, &channels, 3);

	memcpy(tex_data + (w * h - 1) * 3, tex_data, 3);

	glGenTextures(1, &gradient_texture);
	glBindTexture(GL_TEXTURE_1D, gradient_texture);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, w, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

	free(tex_data);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_1D, 0);

	ComputeMandelbrot();
	ColorMandelbrot();
}

MandelbrotMode::~MandelbrotMode() {
	glDeleteProgram(mandelbrot_program);
	glDeleteProgram(mandelbrot_color_program);
	glDeleteProgram(texture_program);

	glDeleteFramebuffers(1, &framebuffer_pos_texture);
	glDeleteFramebuffers(1, &framebuffer_compute_texture);
	glDeleteFramebuffers(1, &framebuffer_color_texture);

	glDeleteVertexArrays(1, &screen_vao);
	glDeleteBuffers(1, &screen_vbo);

	glDeleteTextures(1, &framebuffer_color_texture);
	glDeleteTextures(1, &framebuffer_pos_texture);
	glDeleteTextures(1, &framebuffer_compute_texture);
	glDeleteTextures(1, &gradient_texture);
}

void MandelbrotMode::MessageLoop() {
	Render();
	Render();

	MSG msg{};

	while (true) {
		if (!recompute_flag) MsgWaitForMultipleObjects(0, 0, FALSE, INFINITE, QS_ALLEVENTS);

		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE)) {
			DispatchMessageW(&msg);
			TranslateMessage(&msg);
		}

		if (recompute_flag) {
			ComputeMandelbrot();
			ColorMandelbrot();

			recompute_flag = false;
			recolor_flag = false;
		}
		else if (recolor_flag) {
			ColorMandelbrot();

			recolor_flag = false;
		}

		Render();
	}
}

void draw_texture_to_buffer(vector<unsigned char>& buffer, int b_x, int b_y, int b_w, int b_h, vector<unsigned char>& texture, int width, int height) {
	for (size_t y = b_y, t_y = 0; y < b_y + height; y++, t_y++) {
		memcpy(buffer.data() + (b_w * y + b_x) * 3, texture.data() + (height - 1 - t_y) * width * 3, width * 3);
	}
}

LRESULT CALLBACK MandelbrotMode::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	ImGuiIO io = ImGui::GetIO();

	if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg) {
	case WM_KEYDOWN: {
		if (wParam == 0x45) { // E
			max_iterations = glm::clamp(max_iterations * 1.1 + 1, 1.0, 100000.0);
			recompute_flag = true;
		}
		else if (wParam == 0x51) { // Q
			max_iterations = glm::clamp(max_iterations / 1.1 - 1, 1.0, 100000.0);
			recompute_flag = true;
		}
		else if (wParam == 0x53 && GetKeyState(VK_CONTROL) < 0 && !saved_flag) { // S
			SaveTextureToFile("mandelbrot set ", framebuffer_color_texture, window_width, window_height);

			saved_flag = true;
		}
	} break;
	case WM_LBUTTONDOWN: {
		SetCapture(hwnd);

		select_box_is_active = true;

		select_box_origin = glm::vec2(GET_X_LPARAM(lParam) / (window_width / 2.0) - 1, -(GET_Y_LPARAM(lParam) / (window_height / 2.0) - 1));

		CalculateSelectBox(lParam);
	} break;
	case WM_NCMOUSEMOVE:
	case WM_MOUSEMOVE: {
		if (select_box_is_active) {
			CalculateSelectBox(lParam);
		}
	} break;
	case WM_NCLBUTTONUP:
	case WM_LBUTTONUP: {
		ReleaseCapture();
		select_box_is_active = false;

		if (select_box_size > min_select_box_size) {
			pos += select_box_origin * 2.f / zoom;

			zoom = glm::clamp(zoom / select_box_size, min_zoom, max_zoom);

			recompute_flag = true;
		}

	} break;
	case WM_RBUTTONDOWN: {
		zoom = glm::clamp(zoom / 2, min_zoom, max_zoom);
		recompute_flag = true;
	} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MandelbrotMode::ComputeMandelbrot() {
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, msaa_compute_framebuffer);

	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glUseProgram(mandelbrot_program);

	glUniform1f(max_iteration_uniform_location, max_iterations);
	glUniform1f(power_uniform_location, power);
	glUniform2f(c_uniform_location, c.x, c.y);
	glUniform2f(cz_uniform_location, cz.x, cz.y);
	glUniform2d(pos_uniform_location, pos.x, pos.y);
	glUniform1f(zoom_uniform_location, zoom);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, gradient_texture);

	glBindVertexArray(screen_vao);
	glDrawArrays(GL_QUADS, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MandelbrotMode::ColorMandelbrot() {
	glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glUseProgram(mandelbrot_color_program);

	glUniform1i(res_uniform_location, window_width);
	glUniform1f(grid_density_uniform_location, grid_density);
	glUniform1f(grid_thickness_uniform_location, grid_thickness);
	glUniform3f(grid_color_uniform_location, grid_color[0], grid_color[1], grid_color[2]);
	glUniform1i(draw_grid_uniform_location, draw_grid);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer_pos_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer_compute_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, gradient_texture);

	glBindVertexArray(screen_vao);
	glDrawArrays(GL_QUADS, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	saved_flag = false;
}

void MandelbrotMode::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// draw fractal
	glUseProgram(texture_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer_color_texture);

	glBindVertexArray(screen_vao);

	glDrawArrays(GL_QUADS, 0, 4);

	// draw select box
	if (select_box_is_active) {
		glUseProgram(0); glColor3f(0.5, 0.5, 0.5);
		if (select_box_size < min_select_box_size);
		else if (zoom / select_box_size > max_zoom) glColor3f(0.1, 0.1, 1);
		else glColor3f(1, 0, 1);

		glLineWidth(1);

		glBegin(GL_LINES);

		glm::vec2 top_left_corner = select_box_origin + glm::vec2(-select_box_size, select_box_size);
		glm::vec2 bottom_right_corner = select_box_origin + glm::vec2(select_box_size, -select_box_size);

		glVertex2f(top_left_corner.x, top_left_corner.y);
		glVertex2f(bottom_right_corner.x, top_left_corner.y);

		glVertex2f(bottom_right_corner.x, top_left_corner.y);
		glVertex2f(bottom_right_corner.x, bottom_right_corner.y);

		glVertex2f(bottom_right_corner.x, bottom_right_corner.y);
		glVertex2f(top_left_corner.x, bottom_right_corner.y);

		glVertex2f(top_left_corner.x, top_left_corner.y);
		glVertex2f(top_left_corner.x, bottom_right_corner.y);

		glEnd();
	}

	ImGui::Begin("settings", 0, saved_flag ? 0 : ImGuiWindowFlags_UnsavedDocument);

	if (ImGui::TreeNode("function params")) {
		if (recompute_flag |= ImGui::InputFloat("iterations", &max_iterations, 0, 0, "%f")) {
			max_iterations = glm::clamp(max_iterations, min_max_iterations, max_max_iterations);
			recompute_flag = true;
		}

		recompute_flag |= ImGui::SliderFloat("power", &power, -5, 5, "%f");
		recompute_flag |= ImGui::InputFloat2("position", glm::value_ptr(pos), "%f");
		if (ImGui::InputFloat("zoom", &zoom, 0, 0, "%f")) {
			zoom = glm::clamp(zoom, min_zoom, max_zoom);
			recompute_flag = true;
		}

		if (ImGui::TreeNode("cz")) {
			float r = sqrt(cz.x * cz.x + cz.y * cz.y), fi = atan2(cz.y, cz.x) / (M_PI / 180);

			recompute_flag |= ImGui::SliderFloat("cz.x", &(cz.x), -5, 5, "%f");
			recompute_flag |= ImGui::SliderFloat("cz.y", &(cz.y), -5, 5, "%f");
			if (ImGui::SliderFloat("r", &(r), 0, sqrt(25), "%f")) {
				cz.x = r * cos(fi * (M_PI / 180.0));
				cz.y = r * sin(fi * (M_PI / 180.0));

				recompute_flag = true;
			}
			if (ImGui::SliderFloat("fi", &(fi), -180, 180, "%f")) {
				cz.x = r * cos(fi * (M_PI / 180.0));
				cz.y = r * sin(fi * (M_PI / 180.0));

				recompute_flag = true;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("c")) {
			float r = sqrt(c.x * c.x + c.y * c.y), fi = atan2(c.y, c.x) / (M_PI / 180);

			recompute_flag |= ImGui::SliderFloat("c.x", &(c.x), -5, 5, "%f");
			recompute_flag |= ImGui::SliderFloat("c.y", &(c.y), -5, 5, "%f");
			if (ImGui::SliderFloat("r", &(r), 0, sqrt(25), "%f")) {
				c.x = r * cos(fi * (M_PI / 180.0));
				c.y = r * sin(fi * (M_PI / 180.0));

				recompute_flag = true;
			}
			if (ImGui::SliderFloat("fi", &(fi), -180, 180, "%f")) {
				c.x = r * cos(fi * (M_PI / 180.0));
				c.y = r * sin(fi * (M_PI / 180.0));

				recompute_flag = true;
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("grid")) {
		recolor_flag |= ImGui::Checkbox("draw grid", &draw_grid);
		recolor_flag |= ImGui::SliderFloat("grid density", &grid_density, 0.00001, 1, "%f") && draw_grid;
		recolor_flag |= ImGui::SliderFloat("grid thickness", &grid_thickness, 0.5, 3, "%f") && draw_grid;
		recolor_flag |= ImGui::ColorEdit3("grid color", grid_color) && draw_grid;

		ImGui::TreePop();
	}

	if (ImGui::Button("reset params")) {
		power = 2;
		zoom = 1;
		max_iterations = 35;
		pos = glm::vec2(0, 0);
		cz = glm::vec2(1, 1);
		c = glm::vec2(0, 0);

		draw_grid = false;
		grid_density = 0.1;
		grid_thickness = 1;
		grid_color[0] = 196.0 / 255;
		grid_color[1] = 37.0 / 255;
		grid_color[2] = 37.0 / 255;

		recompute_flag = true;
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SwapBuffers(dc);
}

void MandelbrotMode::CalculateSelectBox(LPARAM lParam) {
	select_box_size = max(abs(GET_X_LPARAM(lParam) / (window_width / 2.0) - 1 - select_box_origin.x), abs((GET_Y_LPARAM(lParam) / (window_height / 2.0) - 1) + select_box_origin.y));

	float distance_to_screen_border = 1 - max(abs(select_box_origin.x), abs(select_box_origin.y));

	select_box_size = min(distance_to_screen_border, select_box_size);
}