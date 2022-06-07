#version 440
out vec3 FragColor;

in vec2 uv;

uniform sampler2D color_ratio_tex;

uniform vec3 color1 = vec3(1, 1, 0);
uniform vec3 color2 = vec3(0, 1, 1);
uniform vec3 color3 = vec3(1, 0, 1);

void main()
{
	vec3 color_ratio = normalize(vec3(texture(color_ratio_tex, uv)));
	FragColor = vec3(color1*color_ratio.x + color2*color_ratio.y + color3*color_ratio.z);
}