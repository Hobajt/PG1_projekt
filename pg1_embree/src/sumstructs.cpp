#include "stdafx.h"
#include "sumstructs.h"

#include "material.h"

IntersectionEmbree SceneData::IntersectRay(RTCRay& ray) {
	IntersectionEmbree data = IntersectionEmbree(SetupRayHitStructure(ray));
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene, &context, &data.rhit);
	ray.tfar = data.rhit.ray.tfar;
	return data;
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

IntersectionEmbree::IntersectionEmbree(RTCRayHit&& _rhit) : rhit(std::move(_rhit)) {
	v_rayDir = { rhit.ray.dir_x, rhit.ray.dir_y, rhit.ray.dir_z };
	v_view = -v_rayDir;
}

void IntersectionEmbree::PrepareData(const SceneData& scene) {
	RTCGeometry geometry = rtcGetGeometry(scene.scene, rhit.hit.geomID);
	material = (Material*)rtcGetGeometryUserData(geometry);

	texDiffuse = material->get_texture(Material::kDiffuseMapSlot);
	rtcInterpolate0(geometry, rhit.hit.primID, rhit.hit.u, rhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &texCoords.u, 2);

	clrEmission = material->emission.AsColor();
	clrAttenuation = material->attenuation.AsColor();

	clrDiffuse = texDiffuse ? texDiffuse->get_texel(texCoords.u, 1 - texCoords.v) : material->diffuse.AsColor();
	clrSpecular = material->specular.AsColor();
	clrAmbient = material->ambient.AsColor();

	//TODO: convert clrs to linear if needed

	//extract normal
	Normal3f n;
	rtcInterpolate0(geometry, rhit.hit.primID, rhit.hit.u, rhit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &n.x, 3);
	v_normal = { n.x, n.y, n.z };
	if (v_normal.DotProduct(v_rayDir) > 0.f) {
		//flips the normal if it's facing away
		v_normal *= -1.f;
	}

	p_rayOrg = { rhit.ray.org_x, rhit.ray.org_y, rhit.ray.org_z };
	p_rayHit = p_rayOrg + (v_rayDir * rhit.ray.tfar);

	v_rayDirReflected = (2.f * dotNormalView) * v_normal - v_view;

	dotNormalRay = v_normal.DotProduct(v_rayDir);
	dotNormalView = -dotNormalRay;
}
