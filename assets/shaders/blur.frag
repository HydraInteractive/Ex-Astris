#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 blurOutput;
layout(location = 1) uniform sampler2D prevImage;
layout(location = 2) uniform bool horizontal;
layout(location = 3) uniform int kernelSize;
uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );
//layout(location = 4) uniform float weight[5];

void main() {
	ivec2 imageSize = textureSize(prevImage, 0);
	blurOutput = vec3(texture(prevImage, vec2(texCoords)) * weight[0]).rgb;
	if(horizontal){
		for(int i = 1; i < 3; i++){
			blurOutput += vec3(texture(prevImage, vec2(texCoords) + vec2(offset[i] / imageSize.x, 0.0)) * weight[i]).rgb;
			blurOutput += vec3(texture(prevImage, vec2(texCoords) - vec2(offset[i] / imageSize.x, 0.0)) * weight[i]).rgb;
		}
	}
	else{
		for(int i = 1; i < 3; i++){
			blurOutput += vec3(texture(prevImage, vec2(texCoords) + vec2(0.0, offset[i] / imageSize.y)) * weight[i]).rgb;
			blurOutput += vec3(texture(prevImage, vec2(texCoords) - vec2(0.0, offset[i] / imageSize.y)) * weight[i]).rgb;
		}
	}

	//vec2 tex_offset = 1.0 / textureSize(prevGlowImage, 0); // gets size of single texel
    //vec3 result = texture(prevGlowImage, texCoords).rgb * weight[0]; // current fragment's contribution
    //if (horizontal) {
    //    for(int i = 1; i < kernelSize; ++i)
    //    {
    //        result += texture(prevGlowImage, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    //        result += texture(prevGlowImage, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    //    }
    //} else {
    //    for(int i = 1; i < kernelSize; ++i)
    //    {
    //        result += texture(prevGlowImage, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    //        result += texture(prevGlowImage, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    //    }
    //}
	//blurOutput = result;
}
