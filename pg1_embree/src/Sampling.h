#pragma once

#include <random>

#include "vector3.h"

//Sampling methods & random generators.
class Sampling {
public:
	static void InitGenerator();
	static inline float Random05() { return dis_n05_p05(generator); }
	static inline float Random1() { return dis_n0_p1(generator); }
private:
	static std::mt19937 generator;
	static std::uniform_real_distribution<float> dis_n05_p05;
	static std::uniform_real_distribution<float> dis_n0_p1;
};

