#pragma once

//#include <embree3/rtcore_ray.h>
#include <embree3/rtcore.h>
#include "vector3.h"

class Material;
class Texture;
struct SceneData;

namespace ShaderType {
	enum {
		Phong = 3,
		Glass = 4,
		Lambert = 5,
		Mirror = 6,
	};
}

//Intersection for embree scene.
struct IntersectionEmbree {
	IntersectionEmbree(RTCRayHit&& rhit);

	//Check if intersection didn't fail before calling this.
	void PrepareData(const SceneData& scene);

	inline bool IntersectionFailed() const { return rhit.hit.geomID == RTC_INVALID_GEOMETRY_ID; }

	bool SurvivedRoulette(float* rho);
public:
	RTCRayHit rhit;

	vec3f v_normal;
	vec3f v_rayDir;
	vec3f v_view;
	vec3f v_rayDirReflected;

	vec3f p_rayHit;
	vec3f p_rayOrg;

	float dotNormalView;
	float dotNormalRay;

	clr3f clrDiffuse;
	clr3f clrSpecular;
	clr3f clrAmbient;

	clr3f clrEmission;
	clr3f clrAttenuation;

	Material* material;
	Texture* texDiffuse;
	Coord2f texCoords;
public:
	static bool convertMaterials;
};

//Wrapper for scene data - cuz i will have to use geometry instead later on.
struct SceneData {
	RTCScene scene;

	IntersectionEmbree IntersectRay(RTCRay& ray);
	static RTCRayHit SetupRayHitStructure(RTCRay& ray);
};