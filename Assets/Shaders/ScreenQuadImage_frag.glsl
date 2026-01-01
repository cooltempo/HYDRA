#version 150

in vec2 vertex_uv;

uniform sampler2D uniform_texture;

out vec4 out_fragColour;

void main()
{
    out_fragColour = texture(uniform_texture, vertex_uv);
}
