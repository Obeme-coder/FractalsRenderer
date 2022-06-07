#include "main.h"
#include "opengl.h"
#include "window.h"

PROGRAM texture_program, mandelbrot_program, sierpinski_piramide_program;
TEXTURE msaa_fbo_texture, fbo_texture;

VBO screen_vbo;
VAO screen_vao;
FBO msaa_fbo, fbo;

float screen_quad_vertices[] = {
	-1, -1,		0, 0,
	1, -1,		1, 0,
	1, 1,		1, 1,
	-1, 1,		0, 1
};

void RenderInit() {
	GLInit();

	texture_program = CreateProgram("texture.vert", "texture.frag");

	// screen triangle_vbo & triangle_vbo
	glGenBuffers(1, &screen_vbo);
	glGenVertexArrays(1, &screen_vao);

	glBindVertexArray(screen_vao);

	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad_vertices), screen_quad_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

SHADER CompileShader(filesystem::path path, GLenum shader_type) {
	// reading shader source code
	ifstream file(path, ifstream::binary | ifstream::ate);
	if (!file.is_open()) {
		cout << "Error: cant open shader file: " << path.string() << endl;
		cin.get();
		exit(-1);
	}
	int filesize = file.tellg();
	file.seekg(0);

	vector<char> _source_code(filesize);
	char* source_code = _source_code.data();
	file.read(source_code, filesize);

	// creating and compiling shader
	SHADER shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &source_code, &filesize);
	glCompileShader(shader);

	// check for errors
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		glDeleteShader(shader);

		cout << "Error: shader compilation error, log: " << errorLog.data();
		cin.get();
		exit(-1);
	}

	return shader;
}

PROGRAM CompileProgram(SHADER v_shader, SHADER f_shader) {
	PROGRAM mandelbrot_program = glCreateProgram();
	glAttachShader(mandelbrot_program, v_shader);
	glAttachShader(mandelbrot_program, f_shader);
	glLinkProgram(mandelbrot_program);

	GLint isLinked = 0;

	glGetProgramiv(mandelbrot_program, GL_LINK_STATUS, &isLinked);
	if (!isLinked) {
		GLint maxLength = 0;
		glGetProgramiv(mandelbrot_program, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(mandelbrot_program, maxLength, &maxLength, &errorLog[0]);

		glDeleteProgram(mandelbrot_program);

		cout << "Error: program linking error: " << errorLog.data() << endl;
		cin.get();
		exit(-1);
	}

	return mandelbrot_program;
}

PROGRAM CreateProgram(filesystem::path v_shader_path, filesystem::path f_shader_path) {
	SHADER v_shader = CompileShader(v_shader_path, GL_VERTEX_SHADER);
	SHADER f_shader = CompileShader(f_shader_path, GL_FRAGMENT_SHADER);
	
	PROGRAM mandelbrot_program = CompileProgram(v_shader, f_shader);

	glDeleteShader(v_shader);
	glDeleteShader(f_shader);

	return mandelbrot_program;
}

void SaveTextureToFile(string base_filename, FBO framebuffer, int width, int height) {
	string filename;

	do {
		filename = base_filename + to_string(rand() % 10000) + ".png";
	} while (filesystem::exists(filename));

	vector<unsigned char> img(width * height * 3);

	if(framebuffer) glGetTextureImage(framebuffer, 0, GL_RGB, GL_UNSIGNED_BYTE, width * height * 3, img.data());
	else glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, img.data());

	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename.c_str(), width, height, 3, img.data(), width * 3);
}