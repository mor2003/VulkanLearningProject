#version 450

layout(location = 0) out vec4 outColors;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoords;

layout(binding = 1) uniform sampler2D texSampler;

void main(){
	outColors = texture(texSampler, texCoords);
}