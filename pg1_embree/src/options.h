#pragma once

#include <string>
#include "vector3.h"

struct Options {
public:
	int width{ 640 };
	int height{ 480 };

	vec3f viewAt{ vec3f(0, 0, 35) };
	vec3f viewFrom{ vec3f(175, -140, 130) };
	float fov{ 45.0 };

	int sampleCount{ 4 };
	int maxDepth{ 10 };

	std::string path_model{"res/models/avenger/RT_6887_allied_avenger.obj"};
	std::string path_background{"res/backgrounds/bg3.hdr"};
public:
	static inline Options& Get() { return instance; }
private:
	static Options Load(std::string_view path);
private:
	static Options instance;
};