#pragma once

#include <random>

#include "vector3.h"

//Container for sampling results.
struct HemisphereSample {
	vec3f omegaI;			//sampled direction of incoming light
	float dotNormalOmegaI;

	float PDF;
	bool invalid;

	float tmp;
};

//Sampling methods & random generators.
class Sampling {
public:
	static void InitGenerator();
	static inline float Random05() { return dis_n05_p05(generator); }
	static inline float Random1() { return dis_n0_p1(generator); }

	//Uniform hemisphere sampling.
	static HemisphereSample Uniform(vec3f& v_normal);

	//Cosine-weighted hemisphere sampling.
	static HemisphereSample CosWeighted(vec3f& v_normal);

	//Cosine-lobe weighted sampling over hemisphere - for specular samples.
	//omegaO -> v_view = outgoing light direction = direction towards previous p_rayHit.
	//gamma -> material shininess.
	static HemisphereSample CosLobe(const vec3f& v_normal, const vec3f& omegaO, float gamma, float& powerCosThetaR);

	static HemisphereSample GGX(const vec3f& v_normal, const vec3f& v_view, float roughness2, vec3f* omegaH, float* D);
private:
	static std::mt19937 generator;
	static std::uniform_real_distribution<float> dis_n05_p05;
	static std::uniform_real_distribution<float> dis_n0_p1;
};

