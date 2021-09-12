#pragma once

//#include <embree3/rtcore_ray.h>
#include <embree3/rtcore.h>
#include "vector3.h"

#include "material.h"

struct SceneData;

struct Geometry;

namespace ShaderType {
	enum {
		Phong = 3,
		Glass = 4,
		Lambert = 5,
		Mirror = 6,
		TorranceSparrow = 7,
	};
}

//Intersection for embree scene.
struct IntersectionData {
	IntersectionData(RTCRayHit&& rhit);

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

	Geometry* g;
public:
	static bool convertMaterials;
};

struct Geometry {
	virtual bool IntersectsRay(IntersectionData& d, float& tfar) = 0;
	virtual vec3f getNormal(const vec3f& pt) = 0;

	Material* mat = Material::GetDefault();
};

struct Sphere : public Geometry {
	float radius;
	vec3f center;

	Sphere(const vec3f& S, float R) : center(S), radius(R) {}

	virtual bool IntersectsRay(IntersectionData& d, float& tfar) override;
	virtual vec3f getNormal(const vec3f& pt) override;
};


struct SceneObject {
	RTCScene scene;

	virtual void PrepareData(IntersectionData& data) = 0;
	virtual IntersectionData IntersectRay(IntersectionData&& data) = 0;
	virtual void PrepareScene(RTCDevice& device_) {}
	virtual void Release() {}
	virtual void Commit() {}
};

struct GeometryScene : public SceneObject {
public:
	void AddGeometry(Geometry* g);
	
	virtual void PrepareData(IntersectionData& data) override;
	virtual IntersectionData IntersectRay(IntersectionData&& data) override;
private:
	std::vector<Geometry*> geometry;
};

struct EmbreeScene : public SceneObject {
	virtual void PrepareData(IntersectionData& data) override;
	virtual IntersectionData IntersectRay(IntersectionData&& data) override;
	virtual void Release() { rtcReleaseScene(scene); }
	virtual void PrepareScene(RTCDevice& device_) override { scene = rtcNewScene(device_); rtcSetSceneFlags(scene, RTC_SCENE_FLAG_ROBUST); }
	virtual void Commit() override { rtcCommitScene(scene); }
};

//Wrapper for scene data - cuz i will have to use geometry instead later on.
struct SceneData {
	SceneData(bool geometry);
	~SceneData() { delete scene; }
	SceneObject* scene;

	IntersectionData IntersectRay(RTCRay& ray);
	static RTCRayHit SetupRayHitStructure(RTCRay& ray);

	void PrepareData(IntersectionData& data);

	void PrepareScene(RTCDevice& device_) { scene->PrepareScene(device_); }
};

//theta - angle between outgoing light and surface normal
float fresnel(float cosTheta, float n1, float n2);

float fresnelSchlick(float cosTheta, float F0);