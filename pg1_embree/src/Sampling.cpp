#include "stdafx.h"
#include "Sampling.h"

std::mt19937 Sampling::generator = std::mt19937();
std::uniform_real_distribution<float> Sampling::dis_n05_p05 = std::uniform_real_distribution<float>(-0.5f, 0.5f);
std::uniform_real_distribution<float> Sampling::dis_n0_p1 = std::uniform_real_distribution<float>(0.f, 1.f);

void Sampling::InitGenerator() {
	generator = std::mt19937(std::random_device{}());;
}
