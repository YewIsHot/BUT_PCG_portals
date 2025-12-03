#version 430 core

uniform mat4 view;
uniform mat4 proj;

void main()
{
    if (gl_VertexID == 0)gl_Position = proj * view * vec4(0, -1, 0, 1);
    if (gl_VertexID == 1)gl_Position = proj * view * vec4(.7, 1, 0, 1);
    if (gl_VertexID == 2)gl_Position = proj * view * vec4(-.7, 1, 0, 1);
}