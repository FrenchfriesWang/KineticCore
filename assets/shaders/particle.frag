#version 460 core
out vec4 FragColor;

in vec2 TexCoord; 
uniform sampler2D particleTexture;
uniform vec4 color;

void main()
{
	FragColor = texture(particleTexture, TexCoord) * color;
}