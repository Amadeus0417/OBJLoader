#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Texcoord;

uniform mat4 MVP;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

out vec3 iPosView;
out vec3 iNormalView;
out vec2 iTexCoord;


void main()
{
    gl_Position = MVP * vec4(Position, 1.0);

    vec4 tmpPos = viewMatrix * worldMatrix * vec4(Position, 1.0);
    iPosView = tmpPos.xyz / tmpPos.w;
    iNormalView = (normalMatrix * vec4(Normal, 0.0)).xyz;
    iNormalView = normalize(iNormalView);
    iTexCoord = Texcoord;
}