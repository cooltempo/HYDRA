#version 150

in vec4 vertex_colour;
in vec2 vertex_uv;

uniform float uniform_particleSmoothness;

out vec4 out_fragColour;

void main()
{
    float r = length(vertex_uv);
    float a = 1.0 - smoothstep(1.0 - uniform_particleSmoothness, 1.0, r);
    out_fragColour = vec4(vertex_colour.rgb, vertex_colour.a * a);
}
