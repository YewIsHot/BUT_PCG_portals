#version 430 core

uniform vec3 dir;
uniform vec4 meshColor;
in vec3 vNormal;
out vec4 fColor;

void main()
{
    float shadingRation = .5f;
    float shading = max(dot(normalize(vNormal), normalize(dir)), 0.1f);

    fColor = 4 * shadingRation * meshColor + (1-shadingRation) * (shading * meshColor);
}
