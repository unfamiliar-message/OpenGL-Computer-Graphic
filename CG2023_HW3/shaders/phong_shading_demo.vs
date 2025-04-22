#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;


uniform mat4 worldMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;

// 要丟到fs的資料
out vec3 iPosWorld;
out vec3 iNormalWorld;
out vec2 iTexCoord;


void main()
{
    // --------------------------------------------------------
    
    gl_Position = MVP * vec4(Position, 1.0);
    vec4 positionTmp = worldMatrix * vec4(Position,1.0);
    iPosWorld = positionTmp.xyz/positionTmp.w;   
    iNormalWorld = (normalMatrix *vec4(Normal,0.0)).xyz;
    iTexCoord = TexCoord;
    // --------------------------------------------------------
}