#include "modes.h"
#include "window.h"

SierpinskiTriangleMode::SierpinskiTriangleMode() {
	// init programs & uniforms
	color_program = CreateProgram(L"sierpinski_triangle_color.vert", L"sierpinski_triangle_color.frag");

	colors_uniform_location[0] = glGetUniformLocation(color_program, "color1");
	colors_uniform_location[1] = glGetUniformLocation(color_program, "color2");
	colors_uniform_location[2] = glGetUniformLocation(color_program, "color3");

	// generate triangle vertices
	vector<float> triangle_vertices(4 * 3 * 3);

	glm::vec2 points[3];
	double angle = M_PI;
	for (size_t i = 0; i < 3; i++, angle += M_PI / 1.5) {
		points[i] = glm::vec2(-sin(angle), -cos(angle));
	}

	for (size_t i = 0; i < 3; i++) {
		glm::vec2 _points[3]{ points[i], points[(i + 1) % 3], points[(i + 2) % 3] };

		for (size_t j = 0; j < 3; j++) {
			triangle_vertices[(i * 3 + j) * 4 + 0] = ((_points[0] + _points[j]) / 2.f).x;
			triangle_vertices[(i * 3 + j) * 4 + 1] = ((_points[0] + _points[j]) / 2.f).y;
			triangle_vertices[(i * 3 + j) * 4 + 2] = (_points[j].x + 1) / 2;
			triangle_vertices[(i * 3 + j) * 4 + 3] = (_points[j].y + 1) / 2;
		}
	}

	// init vbo & vao
	glGenBuffers(1, &triangle_vbo);
	glGenVertexArrays(1, &triangle_vao);

	glBindVertexArray(triangle_vao);

	glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
	glBufferData(GL_ARRAY_BUFFER, triangle_vertices.size() * sizeof(float), triangle_vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// create compute framebuffer
	glGenFramebuffers(1, &msaa_compute_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, msaa_compute_framebuffer);

	glGenTextures(1, &msaa_compute_texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa_compute_texture);

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGB32F, window_width, window_height, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, msaa_compute_texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "fbo error" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create compute framebuffer
	glGenFramebuffers(1, &compute_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, compute_framebuffer);

	glGenTextures(1, &compute_texture);
	glBindTexture(GL_TEXTURE_2D, compute_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, window_width, window_height, 0, GL_RGB, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, compute_texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "fbo error" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create color framebuffer
	glGenFramebuffers(1, &color_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) cout << "fbo error" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// draw first triangle 
	ResetRecursion();
	Color();
}

SierpinskiTriangleMode::~SierpinskiTriangleMode() {
	glDeleteBuffers(1, &triangle_vbo);
	glDeleteVertexArrays(1, &triangle_vao);
}

void SierpinskiTriangleMode::MessageLoop() {
	Render();
	Render();

	MSG msg{};

	while (true) {
		MsgWaitForMultipleObjects(0, 0, FALSE, INFINITE, QS_ALLEVENTS);

		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE)) {
			DispatchMessageW(&msg);
			TranslateMessage(&msg);
		}

		if (recompute_flag) {
			Color();

			recompute_flag = false;
		}

		Render();
	}
}

LRESULT CALLBACK SierpinskiTriangleMode::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
		if (wParam == 0x53 && GetKeyState(VK_CONTROL) < 0 && !saved_flag) { // S
			SaveTextureToFile("mandelbrot set ", color_texture, window_width, window_height);

			saved_flag = true;
		}
		else if (wParam == VK_SPACE) {
			RecursionStep();

			recompute_flag = true;
		}
	} break;
	case WM_LBUTTONDOWN: {
		RecursionStep();

		recompute_flag = true;
	} break;
	case WM_RBUTTONDOWN: {
		ResetRecursion();

		recompute_flag = true;
	} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SierpinskiTriangleMode::ResetRecursion() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearColor(1, 1, 1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, msaa_compute_framebuffer);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearColor(1, 1, 1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(0);
	glBegin(GL_TRIANGLES);

	if (draw_big_triangle_flag) {
		glColor3f(1, 0, 0);
		glVertex2f(sin(0), cos(0));
		glColor3f(0, 1, 0);
		glVertex2f(sin(M_PI / 1.5), cos(M_PI / 1.5));
		glColor3f(0, 0, 1);
		glVertex2f(sin(M_PI / 1.5 * 2), cos(M_PI / 1.5 * 2));
	}

	glColor3f(1, 0, 0);
	glVertex2f(-sin(0) / 2, -cos(0) / 2);
	glColor3f(0, 1, 0);
	glVertex2f(-sin(M_PI / 1.5) / 2, -cos(M_PI / 1.5) / 2);
	glColor3f(0, 0, 1);
	glVertex2f(-sin(M_PI / 1.5 * 2) / 2, -cos(M_PI / 1.5 * 2) / 2);

	glEnd();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_compute_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compute_framebuffer);
	glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	saved_flag = false;
}

void SierpinskiTriangleMode::RecursionStep() {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, msaa_compute_framebuffer);

	glUseProgram(texture_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, compute_texture);

	glBindVertexArray(triangle_vao);

	glDrawArrays(GL_TRIANGLES, 0, 9);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa_compute_framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compute_framebuffer);
	glBlitFramebuffer(0, 0, window_width * 4, window_height * 4, 0, 0, window_width * 4, window_height * 4, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SierpinskiTriangleMode::Color() {
	glViewport(0, 0, window_width, window_height);

	glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

	glUseProgram(color_program);

	glUniform3f(colors_uniform_location[0], colors[0][0], colors[0][1], colors[0][2]);
	glUniform3f(colors_uniform_location[1], colors[1][0], colors[1][1], colors[1][2]);
	glUniform3f(colors_uniform_location[2], colors[2][0], colors[2][1], colors[2][2]);

	glBindVertexArray(screen_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, compute_texture);

	glDrawArrays(GL_QUADS, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	saved_flag = false;
}

void SierpinskiTriangleMode::Render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(texture_program);

	glBindVertexArray(screen_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, color_texture);

	glDrawArrays(GL_QUADS, 0, 4);

	// ImGui window
	ImGui::Begin("settings", 0, (saved_flag ? 0 : ImGuiWindowFlags_UnsavedDocument) | ImGuiWindowFlags_NoBackground);

	ImGui::Checkbox("draw big triangle", &draw_big_triangle_flag);

	const ImGuiColorEditFlags color_picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB;
	recompute_flag |= ImGui::ColorPicker3("##color1", (float*)&colors[0], color_picker_flags);
	recompute_flag |= ImGui::ColorPicker3("##color2", (float*)&colors[1], color_picker_flags);
	recompute_flag |= ImGui::ColorPicker3("##color3", (float*)&colors[2], color_picker_flags);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SwapBuffers(dc);
}