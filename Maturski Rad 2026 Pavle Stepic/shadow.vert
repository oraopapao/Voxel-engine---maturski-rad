#version 460 core

layout(location = 0) in uint packedData;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	uint x = packedData & 31u;
    uint y = (packedData >> 5u) & 511u;
    uint z = (packedData >> 14u) & 31u;

	vec3 pos = vec3(float(x), float(y), float(z));
	gl_Position = lightSpaceMatrix * model * vec4(pos, 1.0f);
}