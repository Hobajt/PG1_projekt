#pragma once

//#include <embree3/rtcore_ray.h>
#include <embree3/rtcore.h>

//Wrapper for scene data - cuz i will have to use geometry instead later on.
struct SceneData {
	RTCScene scene;
};