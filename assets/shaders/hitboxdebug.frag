#version 440 core

layout(location = 0) out vec3 fragOutput;

in vec3 lineColor;

void main() {
	fragOutput = lineColor; 
}