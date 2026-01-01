#version 150

// Per vertex
in vec2 in_uv;

// Per instance
in vec3 in_particlePosition;
in vec4 in_colour;
in float in_particleSize;

uniform mat4 uniform_modelViewMatrix;
uniform mat4 uniform_projectionMatrix;
uniform float uniform_backgroundFadeCoefficient;

out vec4 vertex_colour;
out vec2 vertex_uv;
 
void main()
{
    vertex_colour = in_colour;
    vertex_uv = in_uv;

    vec4 cameraSpacePos = uniform_modelViewMatrix * vec4(in_particlePosition, 1.0);
    cameraSpacePos.xy += in_uv * in_particleSize; // billboard effect

    vec4 cameraSpace0 = uniform_modelViewMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    float fadeZ = cameraSpacePos.z - cameraSpace0.z;
    float fadeAlpha = 1.0 + fadeZ * uniform_backgroundFadeCoefficient;
    vertex_colour.a *= clamp(fadeAlpha, 0.0, 1.0);

    gl_Position = uniform_projectionMatrix * cameraSpacePos;
}
