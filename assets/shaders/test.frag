#version 420 core

out vec4 fragmentColor;

in vec3 fragColor;

void main()
{
	fragmentColor = vec4(fragColor, 1.0);
}