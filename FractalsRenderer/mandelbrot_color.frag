#version 440
out vec3 FragColor;

in vec2 uv;

layout(binding=0)uniform sampler2D pos_tex;
layout(binding=1)uniform sampler2D iter_tex;
layout(binding=2)uniform sampler1D gradient;

uniform int res;
uniform float grid_density = 0.1;
uniform float grid_thickness = 0.8;
uniform vec3 grid_color = vec3(1, 1, 0.1);
uniform bool draw_grid = false;

void main()
{
    float half_p = 1.0 / res;
    vec2 density = vec2(length(texture(pos_tex, uv + vec2(half_p, 0)) - texture(pos_tex, uv - vec2(half_p, 0))), length(texture(pos_tex, uv + vec2(0, half_p)) - texture(pos_tex, uv - vec2(0, half_p))));
  
    FragColor = vec3(texture(gradient, float(texture(iter_tex, uv))));
    
    if(draw_grid) {
        float dist_to_grid = min(abs(mod(vec2(texture(pos_tex, uv)).x + grid_density/2, grid_density)-grid_density / 2) / density.x, abs(mod(vec2(texture(pos_tex, uv)).y + grid_density/2, grid_density)-grid_density / 2) / density.y);
        
        if(grid_thickness/2 - dist_to_grid > 0.5) {
            FragColor = grid_color;
        }
        else if(dist_to_grid <= grid_thickness/2) {
            FragColor = mix(FragColor, grid_color, sqrt((grid_thickness/2 - dist_to_grid)*2));
        }
    }
}