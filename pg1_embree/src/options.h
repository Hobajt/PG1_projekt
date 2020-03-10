#pragma once

#include <string>

struct Options {
public:
	int width{ 640 };
	int height{ 480 };

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