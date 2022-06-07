#include "modes.h"
#include "window.h"

MenuMode::MenuMode() {
	glViewport(0, 0, window_width, window_height);
}

MenuMode::~MenuMode() {

}

void MenuMode::MessageLoop() {
	MSG msg{};

	while (true) {
		MsgWaitForMultipleObjects(0, 0, FALSE, INFINITE, QS_ALLEVENTS);

		while (PeekMessageW(&msg, window, 0, 0, PM_REMOVE)) {
			if (msg.message == exit_message_loop_wm) return;

			DispatchMessageW(&msg);
			TranslateMessage(&msg);
		}

		Render();
	}
}

LRESULT CALLBACK MenuMode::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	ImGuiIO io = ImGui::GetIO();

	if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	if (msg == WM_PAINT) {
		cout << "r" << endl;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MenuMode::Render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGui::Begin("menu", 0, flags);

	ImGui::Text("choose mode:");

	if (ImGui::Button("mandelbrot")) {
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SwapBuffers(dc);

		mode = unique_ptr<Mode>((Mode*)new MandelbrotMode());
		PostMessageW(window, exit_message_loop_wm, 0, 0);
		return;
	}
	else if (ImGui::Button("sierpinski")) {
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SwapBuffers(dc);

		mode = unique_ptr<Mode>((Mode*)new SierpinskiTriangleMode());
		PostMessageW(window, exit_message_loop_wm, 0, 0);
		return;
	}


	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SwapBuffers(dc);
}