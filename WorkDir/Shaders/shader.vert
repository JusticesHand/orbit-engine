#version 450
#extension GL_ARB_separate_shader_objects : enable

// Vertices
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;

// Instanced input
layout(location = 4) in mat4 inModel;

// Uniform buffer
layout(binding = 0) uniform UBO
{
	mat4 viewProjection;
} ubo;

// Out
layout(location = 0) out vec2 outUv;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outColor;

void main() {
	gl_Position = ubo.viewProjection * inModel * vec4(inPosition, 1.0);
	outUv = inUv;
	outNormal = inNormal;
	outColor = inColor;
}