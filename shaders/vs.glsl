#version 330 core

// input vertex data, different for all executions of this shader.
layout( location = 0 ) in vec3 vertexPosition;
layout( location = 1 ) in vec3 vertexNormal;
layout( location = 2 ) in vec2 vertexTexture;

uniform mat4 perspectiveMat;
uniform mat4 lookAtMat;
uniform mat4 transformMat;

uniform mat4 normalMat;

uniform int tFlag;

varying vec3 position;
varying vec3 normal;
varying vec2 texturePosition;
void main() {
    gl_Position = perspectiveMat * lookAtMat * transformMat * vec4(vertexPosition,1.0);
	normal = vec3(normalize(normalMat * vec4(vertexNormal,0.0)));
	position = gl_Position.xyz;
	if(tFlag == 1)
		texturePosition = vec2(vertexTexture.x, 1 - vertexTexture.y);
}