#version 440
layout(location = 0) in vec2 aPos;
out vec2 uv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0, 1.0);
    uv = aPos*2;
}