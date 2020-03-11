#include "stdafx.h"
#include "Sampling.h"

#include "matrix3x3.h"

std::mt19937 Sampling::generator = std::mt19937();
std::uniform_real_distribution<float> Sampling::dis_n05_p05 = std::uniform_real_distribution<float>(-0.5f, 0.5f);
std::uniform_real_distribution<float> Sampling::dis_n0_p1 = std::uniform_real_distribution<float>(0.f, 1.f);

void Sampling::InitGenerator() {
	generator = std::mt19937(std::random_device{}());;
}

HemisphereSample Sampling::Uniform(vec3f& v_normal) {
	HemisphereSample sample{};

	//random values
	float xi1 = (float)(Random1() * 2.f * M_PI);
	float xi2 = Random1();

	//sample random direction in the hemisphere
	float sqrtVal = 2.f * sqrtf(xi2 * (1 - xi2));
	sample.omegaI = { cosf(xi1) * sqrtVal, sinf(xi1) * sqrtVal, 1 - 2.f * xi2 };

	//PDF for uniform hemisphere sampling
	float PDF = _1_2PI;

	//flip the vector if it isn't in the right hemisphere
	sample.dotNormalOmegaI = v_normal.DotProduct(sample.omegaI);
	if (sample.dotNormalOmegaI < 0.f) {
		sample.omegaI = -sample.omegaI;
		sample.dotNormalOmegaI = -sample.dotNormalOmegaI;
	}

	return sample;
}

HemisphereSample Sampling::CosWeighted(vec3f& v_normal) {
	HemisphereSample sample{};

	float xi1 = (float)(Random1() * 2.f * M_PI);
	float xi2 = Random1();

	float sqrtVal = sqrtf(1 - xi2);

	//change-of-basis matrix
	vec3f o1 = v_normal.Orthogonal().normalize();
	vec3f o2 = o1.cross(v_normal).normalize();
	Matrix3 mat = { o2, o1, v_normal };

	//create sample
	sample.omegaI = { cosf(xi1) * sqrtVal, sinf(xi1) * sqrtVal, sqrtf(xi2) };

	//PDF = cos(theta) / PI; theta = polar angle = acos(omegaI.z)
	sample.PDF = sample.omegaI.z * (float)M_1_PI;

	//convert sample into the world space
	sample.omegaI = (mat * sample.omegaI).normalize();

	sample.dotNormalOmegaI = v_normal.DotProduct(sample.omegaI);
	return sample;
}

HemisphereSample Sampling::CosLobe(const vec3f& v_normal, const vec3f& omegaO, float gamma, float& powerCosThetaR) {
	HemisphereSample sample{};

	float xi1 = Sampling::Random1() * 2.f * (float)M_PI;						//2*PI*xi1
	float xi2 = Sampling::Random1();

	float _1_gamma = 1 / (gamma + 1);
	float sqrtVal = sqrtf(1 - powf(xi2, 2.f * _1_gamma));						//sqrt(1 - xi2^(2/(gamma+1)) )

	//reflection of omegaO - serves as new normal for the lobe.
	vec3f omegaR = v_normal.ReflectVector(omegaO).normalize();

	//change-of-basis matrix.
	vec3f o1 = omegaR.Orthogonal().normalize();
	vec3f o2 = o1.cross(omegaR).normalize();
	Matrix3 mat = { o2, o1, omegaR };

	//sample omegaI and convert it into the world space
	sample.omegaI = { cosf(xi1) * sqrtVal, sinf(xi1) * sqrtVal, powf(xi2, _1_gamma) };
	sample.omegaI = (mat * sample.omegaI).normalize();

	sample.dotNormalOmegaI = omegaR.DotProduct(sample.omegaI);

	//float powerCosThetaR = powf(sample.omegaI.DotProduct(omegaR), gamma);
	powerCosThetaR = powf(sample.dotNormalOmegaI, gamma);

	//pdf value
	sample.PDF = (gamma + 2) * (float)_1_2PI;

	//zeroes out all samples that are going underneath the surface
	sample.invalid = v_normal.DotProduct(sample.omegaI) <= 0;

	/*if (isinf(sample._1_pdf)) {
		printf("adsfasdfasd\n");
	}*/

	return sample;
}