
#version 460 core

out vec4 FragColor;
in vec2 texCoord;

flat in vec3 normal;
flat in uint texLayer;

uniform sampler2DArray tex0;
uniform vec3 lightDir;



void main()
{

    float ambient = 0.72;
    float diffuse = max(dot(normal, normalize(lightDir)), 0.0);
    float light = min((ambient + diffuse), 0.92);

    vec4 texColor = texture(tex0, vec3(texCoord, float(texLayer)));
    FragColor = vec4(texColor.rgb * light, texColor.a);
}