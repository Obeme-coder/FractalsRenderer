#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include "glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;

typedef unsigned int VBO, VAO, EBO, FBO;
typedef unsigned int SHADER, PROGRAM, TEXTURE;
typedef unsigned char COLOR[3];

extern PROGRAM texture_program, mandelbrot_program, sierpinski_piramide_program;
extern TEXTURE msaa_fbo_texture, fbo_texture;

extern VBO screen_vbo;
extern VAO screen_vao;
extern FBO msaa_fbo, fbo;
  
extern float screen_quad_vertices[16];

void RenderInit();
SHADER CompileShader(filesystem::path filename, GLenum shader_type);
PROGRAM CompileProgram(SHADER v_shader, SHADER f_shader);
PROGRAM CreateProgram(filesystem::path v_shader_path, filesystem::path f_shader_path);
void SaveTextureToFile(string base_filename, FBO framebuffer, int width, int height);