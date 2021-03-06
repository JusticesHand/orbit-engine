#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUv;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;

// Texture sampler
layout(binding = 1) uniform sampler2D texSampler;

void main() {
	outColor = inColor * texture(texSampler, inUv);
}