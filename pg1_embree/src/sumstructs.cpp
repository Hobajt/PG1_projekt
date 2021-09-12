#include "stdafx.h"
#include "sumstructs.h"

#include "material.h"
#include "Sampling.h"

bool IntersectionData::convertMaterials = false;

SceneData::SceneData(bool geometry) {
	if (geometry) {
		auto gs = new GeometryScene();
		gs->AddGeometry(new Sphere(vec3f{0.f, 0.f, 0.f}, 1.f));
		scene = gs;
	}
	else
		scene = new EmbreeScene();
}

IntersectionData SceneData::IntersectRay(RTCRay& ray) {
	return scene->IntersectRay(IntersectionData(SetupRayHitStructure(ray)));
}

RTCRayHit SceneData::SetupRayHitStructure(RTCRay& ray) {
	RTCHit hit = RTCHit();
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.primID = RTC_INVALID_GEOMETRY_ID;
	hit.Ng_x = hit.Ng_y = hit.Ng_z = 0.f;

	RTCRayHit rhit;
	rhit.hit = hit;
	rhit.ray = ray;

	return rhit;
}

void SceneData::PrepareData(IntersectionData& data) {
	scene->PrepareData(data);
}

IntersectionData::IntersectionData(RTCRayHit&& _rhit) : rhit(std::move(_rhit)) {
	v_rayDir = { rhit.ray.dir_x, rhit.ray.dir_y, rhit.ray.dir_z };
	p_rayOrg = { rhit.ray.org_x, rhit.ray.org_y, rhit.ray.org_z };
	v_view = -v_rayDir;
}

void EmbreeScene::PrepareData(IntersectionData& d) {
	RTCGeometry geometry = rtcGetGeometry(scene, d.rhit.hit.geomID);
	d.material = (Material*)rtcGetGeometryUserData(geometry);

	d.texDiffuse = d.material->get_texture(Material::kDiffuseMapSlot);
	rtcInterpolate0(geometry, d.rhit.hit.primID, d.rhit.hit.u, d.rhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &d.texCoords.u, 2);

	d.clrEmission = d.material->emission.AsColor();
	d.clrAttenuation = d.material->attenuation.AsColor();

	d.clrDiffuse = d.texDiffuse ? d.texDiffuse->get_texel(d.texCoords.u, 1 - d.texCoords.v) : d.material->diffuse.AsColor();
	d.clrSpecular = d.material->specular.AsColor();
	d.clrAmbient = d.material->ambient.AsColor();

	if (d.convertMaterials) {
		d.clrDiffuse.AsLinear();
		d.clrSpecular.AsLinear();
		d.clrAmbient.AsLinear();
	}

	//extract normal
	Normal3f n;
	rtcInterpolate0(geometry, d.rhit.hit.primID, d.rhit.hit.u, d.rhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &n.x, 3);
	d.v_normal = { n.x, n.y, n.z };
	if (d.v_normal.DotProduct(d.v_rayDir) > 0.f) {
		//flips the normal if it's facing away
		d.v_normal *= -1.f;
	}

	
	d.p_rayHit = d.p_rayOrg + (d.v_rayDir * d.rhit.ray.tfar);


	d.dotNormalRay = d.v_normal.DotProduct(d.v_rayDir);
	d.dotNormalView = -d.dotNormalRay;

	d.v_rayDirReflected = (2.f * d.dotNormalView) * d.v_normal - d.v_view;
}

bool IntersectionData::SurvivedRoulette(float* rho) {
	*rho = 1.f;

	switch (material->shader) {
		case ShaderType::Glass:
		case ShaderType::Mirror:
			return true;
		case ShaderType::Lambert:
			*rho = clrDiffuse.LargestValue();
			break;
		case ShaderType::Phong:
			*rho = max(clrDiffuse.LargestValue(), clrSpecular.LargestValue());
			break;
	}

	return Sampling::Random1() <= *rho;
}


float fresnel(float cosTheta, float n1, float n2) {
	float iorRatio = n1 / n2;
	float F0 = (n1 - n2) / (n1 + n2); F0 *= F0;
	return fresnelSchlick(cosTheta, F0);
}

float fresnelSchlick(float cosTheta, float F0) {
	float minTheta = 1 - cosTheta;
	return F0 + (1 - F0) * minTheta * minTheta * minTheta * minTheta * minTheta;
}

void GeometryScene::AddGeometry(Geometry* g) {
	geometry.push_back(g);
}

void GeometryScene::PrepareData(IntersectionData& d) {
	d.material = d.g->mat;

	d.clrEmission = d.material->emission.AsColor();
	d.clrAttenuation = d.material->attenuation.AsColor();

	d.clrDiffuse = d.material->diffuse.AsColor();
	d.clrSpecular = d.material->specular.AsColor();
	d.clrAmbient = d.material->ambient.AsColor();

	if (d.convertMaterials) {
		d.clrDiffuse.AsLinear();
		d.clrSpecular.AsLinear();
		d.clrAmbient.AsLinear();
	}

	d.p_rayHit = d.p_rayOrg + (d.v_rayDir * d.rhit.ray.tfar);

	//extract normal
	d.v_normal = d.g->getNormal(d.p_rayHit);

	//if(d.v_normal)

	d.dotNormalRay = d.v_normal.DotProduct(d.v_rayDir);
	d.dotNormalView = -d.dotNormalRay;

	d.v_rayDirReflected = (2.f * d.dotNormalView) * d.v_normal - d.v_view;
}

IntersectionData GeometryScene::IntersectRay(IntersectionData&& data) {
	float closest = INFINITY;
	data.g = nullptr;
	for (auto& g : geometry) {
		float tfar = INFINITY;
		if (g->IntersectsRay(data, tfar)) {

			if (tfar < closest) {
				closest = tfar;
				data.g = g;
				data.rhit.ray.tfar = closest;
			}
		}
	}

	data.rhit.hit.geomID =  data.g == nullptr ? RTC_INVALID_GEOMETRY_ID : 1;

	return data;
}

IntersectionData EmbreeScene::IntersectRay(IntersectionData&& data) {
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene, &context, &data.rhit);
	return data;
}

bool Sphere::IntersectsRay(IntersectionData& d, float& tfar) {
	vec3f diff = d.p_rayOrg - center;

	//params of quadratic equation ax^2 + bx + c = 0
	float a = 1.f;			// == ||ray.dir||^2 - always 1 cuz dir is a unit vector.
	float b = diff.DotProduct(d.v_rayDir) * 2.f;
	float c = diff.SqrL2Norm() - (radius * radius);

	//find discriminant, terminate if equation has no roots
	float discr = b * b - 4 * a * c;
	if (discr < 0.f)
		return false;

	//calculate equation's roots
	float _1_2a = 1.f / (2.f * a);
	float sqrDiscr = sqrtf(discr);

	float x1 = (-b + sqrDiscr) * _1_2a;
	float x2 = (-b - sqrDiscr) * _1_2a;

	tfar = min(x1, x2);
	if (tfar < d.rhit.ray.tnear)		//TODO: might cause trouble (when x1 is negative but x2 positive)
		return false;

	return true;
}

vec3f Sphere::getNormal(const vec3f& pt) {
	return (pt - center).normalize();
}
