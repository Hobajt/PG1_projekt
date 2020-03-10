#include "stdafx.h"
#include "Background.h"
#include "texture.h"

#define _USE_MATH_DEFINES
#include <math.h>

float Background::M_1_2PI = (float)(1.f / (2 * M_PI));

BackgroundSkydome::BackgroundSkydome(const std::string& texturePath) : texture(std::make_unique<Texture>(texturePath.c_str())) {}

clr3f BackgroundSkydome::GetPixelColor(vec3f& rayDir) const {
	float phi = (float)(-atan2f(rayDir.y, rayDir.x) + M_PI);
	float theta = acosf(rayDir.z);
	Coord2f bgCoords = { phi * M_1_2PI, theta * (float)M_1_PI };


	return texture->get_texel(bgCoords.u, bgCoords.v);
}

BackgroundSkybox::BackgroundSkybox(const std::string& texturePathBase) {
	char nameEnd[2][5] = { {'p','o','s',' ','\0'}, {'n','e','g',' ','\0'} };
	char* s;
	std::string str;

	for (int i = 0; i < textures.size(); i++) {
		s = nameEnd[i / 3];
		s[3] = ('x' + i % 3);

		str = (texturePathBase + (const char*)s + ".jpg");
		textures[i] = std::make_unique<Texture>(str.c_str());
		//printf("%s\n", str.c_str());
	}
}

clr3f BackgroundSkybox::GetPixelColor(vec3f& rayDir) const {
	int axisIdx = rayDir.LargestComponent(true);
	int textureIdx = rayDir[axisIdx] >= 0 ? axisIdx : axisIdx + 3;

	const float tmp = 1.f / abs(rayDir[axisIdx]);
	float u = (rayDir[axisIdx + 1] * tmp + 1) * 0.5f;
	float v = (rayDir[axisIdx + 2] * tmp + 1) * 0.5f;

	float temp;
	switch (textureIdx) {
		case 0:
			v = 1 - v;
			u = 1 - u;
			break;
		case 1:
			temp = u;
			u = v;
			v = 1 - temp;
			break;
		case 2:
			//v = 1 - v;
			//u = 1 - u;
			break;
		case 3:
			v = 1 - v;
			break;
		case 4:
			temp = u;
			u = 1 - v;
			v = 1 - temp;
			break;
		case 5:
			v = 1 - v;
			//u = 1 - u;
			break;
	}

	return textures[textureIdx]->get_texel(u, v);
}

BackgroundStatic::BackgroundStatic(const clr3f& _color) : color(_color) {}

clr3f BackgroundStatic::GetPixelColor(vec3f& rayDir) const {
	return color;
}
