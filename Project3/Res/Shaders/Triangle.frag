#version 450

layout(location = 0) out vec4 outColors;
layout(location = 0) in vec3 fragColor;

void main(){
	outColors = vec4(fragColor, 1.0f);
}