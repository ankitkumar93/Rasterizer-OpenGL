#version 330 core
precision highp float;
precision highp int;
varying vec3 normal;
varying vec3 position;
varying vec2 texturePosition;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float n;

uniform int tFlag;
uniform sampler2D texture;

out vec4 frag_Color;

void main() {
	vec3 eye = vec3(0,0,-30);
	vec3 light = vec3(0,0,10);
	vec3 vVector = normalize(eye - position);
	vec3 lVector = normalize(light - position);
	vec3 hVector = normalize(vVector + lVector);

	float diffFactor = max(dot(lVector, normal), 0.0);
	float specFactor = 0.0;
	if(diffFactor > 0.0)
		specFactor = max(dot(hVector, normal), 0.0);

	specFactor = pow(specFactor, n);

	vec3 color = ka + kd*diffFactor + ks*specFactor;

	if(tFlag == 1){
		vec4 textureData = texture2D(texture, texturePosition);
		if(textureData.a < 1.0){
			color = vec3(1.0,1.0,0.0);
		}else{
			color = color * textureData.rgb;
			}
	}

	frag_Color = vec4(color, 1.0);
}