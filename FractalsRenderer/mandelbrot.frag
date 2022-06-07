#version 440
in vec2 uv;
layout(location = 0) out vec2 Fragment0;
layout(location = 1) out float  Fragment1;

uniform float max_iterations;
uniform float zoom;
uniform dvec2 pos;
uniform float power = 2;
uniform vec2 cz;
uniform vec2 c;

uniform sampler1D gradient;

void main()
{
    dvec2 d_pos = dvec2(uv)/zoom + pos;

    double real = d_pos.x;
    double imag = d_pos.y;
    double prev_real = real;
    double prev_imag = imag;
    
    double len = sqrt(real * real + imag * imag);
    int iterations = 0;

    while (iterations <= max_iterations)
    {
        double angle = atan(float(prev_imag), float(prev_real));  
        
        len = pow(float(len), power);
        
        real = len * cos(float(angle*power));
        imag = len * sin(float(angle*power));

        real += d_pos.x * cz.x + c.x;
        imag += d_pos.y * cz.y + c.y;
        
        prev_real = real;
        prev_imag = imag;

        len = sqrt(real * real + imag * imag);

        ++iterations;
        
        if (len > 2.0f) {
            break;
        }
    }

    Fragment0 = vec2(real, imag);
    Fragment1 = iterations / max_iterations;
}