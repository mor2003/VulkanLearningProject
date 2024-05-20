#version 450

layout(binding = 0) uniform UniformBufferObject{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec2 Position;
layout(location = 1) in vec3 color;

layout (location = 0) out vec3 fragColor;

void main(){
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(Position, 0.0f, 1.0f);
	fragColor = color;
}