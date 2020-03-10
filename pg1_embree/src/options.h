#pragma once

#include <string>
#include "vector3.h"

struct Options {
public:
	int width{ 640 };
	int height{ 480 };

	Vector3 viewAt{ Vector3(0, 0, 35) };
	Vector3 viewFrom{ Vector3(175, -140, 130) };
	float fov{ 45.0 };

	int sampleCount{ 4 };
	int maxDepth{ 10 };

	std::string path_model{"res/models/RT_6887_allied_avenger.obj"};
	std::string path_background{"res/backgrounds/bg3.hdr"};
public:
	static inline Options& Get() { return instance; }
private:
	static Options Load(std::string_view path);
private:
	static Options instance;
};