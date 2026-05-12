#version 410 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNor;
layout (location = 2) in vec2 vertTexCoord;
layout (location = 3) in uvec4 boneIds;
layout (location = 4) in vec4 boneWeights;

const int MAX_BONES = 100;

uniform mat4 model;
uniform int isSkinned;
uniform mat4 bones[MAX_BONES];

out vec3 fragPos;
out vec3 fragNor;
out vec2 fragTexCoord;

layout (std140) uniform CameraData
{
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
};

void main()
{
    mat4 skin = mat4(1.0);

    if (isSkinned == 1)
    {
        skin =
            bones[boneIds.x] * boneWeights.x +
            bones[boneIds.y] * boneWeights.y +
            bones[boneIds.z] * boneWeights.z +
            bones[boneIds.w] * boneWeights.w;
    }

    vec4 worldPos = model * skin * vec4(vertPos, 1.0);

    gl_Position = projection * view * worldPos;
    fragPos = worldPos.xyz;
    fragNor = mat3(transpose(inverse(model * skin))) * vertNor;
    fragTexCoord = vertTexCoord;
}