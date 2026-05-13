#version 410 core

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTexCoord;

out vec4 color;

uniform sampler2D splat0;

uniform sampler2D terrainGrass;
uniform sampler2D terrainSand;
uniform sampler2D terrainRock;
uniform sampler2D terrainSnow;

uniform float terrainTextureTiling;

void main()
{
    vec4 weights = texture(splat0, fragTexCoord);

    float total = weights.r + weights.g + weights.b + weights.a;
    if (total > 0.0001)
        weights /= total;

    vec2 uv = fragTexCoord * terrainTextureTiling;

    vec3 grass = texture(terrainGrass, uv).rgb;
    vec3 sand  = texture(terrainSand, uv).rgb;
    vec3 rock  = texture(terrainRock, uv).rgb;
    vec3 snow  = texture(terrainSnow, uv).rgb;

    vec3 blended =
        grass * weights.r +
        sand  * weights.g +
        rock  * weights.b +
        snow  * weights.a;

    color = vec4(blended, 1.0);
}