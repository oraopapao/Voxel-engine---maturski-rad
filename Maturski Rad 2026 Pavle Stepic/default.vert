
#version 460 core

layout(location = 0) in uint packedData;

out vec2 texCoord;
flat out uint texLayer;

uniform mat4 camMatrix;
uniform mat4 model;

flat out vec3 normal;

const vec2 uvs[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

const vec3 faces[6] = vec3[6](
	vec3( 0,  1,  0),
	vec3( 0, -1,  0),
	vec3( 0,  0,  1),
	vec3( 0,  0, -1),
	vec3( 1,  0,  0),
	vec3(-1,  0,  0)
);


void main()
{
	uint x = packedData & 31u;
	uint y = (packedData >> 5u) & 511u;
	uint z = (packedData >> 14u) & 31u;
	uint face = (packedData >> 19u) & 7u;
	uint corner = (packedData >> 22u) & 3u;
	uint tex = (packedData >> 24u) & 255u;

	vec3 pos = vec3(float(x), float(y), float(z));

	gl_Position = camMatrix * model * vec4(pos, 1.0);

	texLayer = tex;
	texCoord = uvs[corner];
	normal = faces[face];

}