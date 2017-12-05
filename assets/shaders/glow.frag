#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 finalOutput;

layout(location = 1) uniform sampler2D originalImage;
layout(location = 2) uniform sampler2D blurrImage;
layout(location = 3) uniform bool glowEnabled;
layout(location = 4) uniform sampler2D depth;
layout(location = 5) uniform vec2 screenSize;

const float quality[7] = {1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0};
const float SUBPIXEL_QUALITY = 0.75;

vec3 getColor(vec2 texCoords, ivec2 offset) {
	vec3 hdrColor = textureOffset(originalImage, texCoords, offset).rgb;
	vec3 bloomColor = textureOffset(blurrImage, texCoords, offset).rgb;

	return hdrColor ;
}


void main() {

	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MIN = 1.0 / 128.0;
	float FXAA_REDUCE_MUL = 1.0 / 32.0;

	vec3 luma = vec3(0.299, 0.587, 0.114);
	vec2 texCoordOffset;
	texCoordOffset.x = 1.0 / screenSize.x;
	texCoordOffset.y = 1.0 / screenSize.y;

	float lumaTL = dot(luma, texture(originalImage, texCoords + (vec2(-1.0, -1.0) * texCoordOffset)).xyz);
	float lumaTR = dot(luma, texture(originalImage, texCoords + (vec2(1.0, -1.0) * texCoordOffset)).xyz);
	float lumaBL = dot(luma, texture(originalImage, texCoords + (vec2(-1.0, 1.0) * texCoordOffset)).xyz);
	float lumaBR = dot(luma, texture(originalImage, texCoords + (vec2(1.0, 1.0) * texCoordOffset)).xyz);
	float lumaM = dot(luma, texture(originalImage, texCoords).xyz);

	vec2 dir;
	dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
	dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));
	
	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
	float inverseDirAdjustment = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
		max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdjustment)) * texCoordOffset;

	vec3 result1 = (1.0 / 2.0) * ( 
		texture(originalImage, texCoords + (dir * vec2(1.0 / 3.0 - 0.5))).xyz + 
		texture(originalImage, texCoords + (dir * vec2(2.0 / 3.0 - 0.5))).xyz);

	vec3 result2 = result1 * (1.0 / 2.0) + (1.0 / 4.0) * (
		texture(originalImage, texCoords + (dir * vec2(0.0 / 3.0 - 0.5))).xyz + 
		texture(originalImage, texCoords + (dir * vec2(3.0 / 3.0 - 0.5))).xyz);

	float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	float lumaResult2 = dot(luma, result2);

	if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		finalOutput = result1;
	else
		finalOutput = result2;

	//vec3 hdrColor = texture(originalImage, texCoords).rgb;
	vec3 bloomColor = texture(blurrImage, texCoords).rgb;

	//hdrColor += bloomColor;

	gl_FragDepth = texture(depth, texCoords).r;
	finalOutput += bloomColor;
}
