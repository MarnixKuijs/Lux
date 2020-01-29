#version 460

in vec2 outUV;
uniform sampler2D image;

out vec4 fragColor;

void main() 
{
	fragColor = texture(image, UV);
}