#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in uint modelPtr;
layout(std430, binding = 0) readonly buffer Doors { mat4 doorModels[]; };
layout(std430, binding = 1) readonly buffer Walls { mat4 wallModels[]; };
layout(std430, binding = 2) readonly buffer Floor { mat4 floorModels[]; };

out vec3 vNormal;

uniform int modelType;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    mat4 model;
    if (modelType == 0)
        model = doorModels[modelPtr];
    else if (modelType == 1)
        model = wallModels[modelPtr];
    else
        model = floorModels[modelPtr];

    gl_Position = proj * view * model * vec4(pos, 1.f);

    mat3 rotationMat = mat3(
        model[0][0], model[0][1], model[0][2],
        model[1][0], model[1][1], model[1][2],
        model[2][0], model[2][1], model[2][2]
        );

    vNormal = rotationMat * normal;
    
}
