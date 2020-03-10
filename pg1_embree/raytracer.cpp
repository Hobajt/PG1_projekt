#include "stdafx.h"
#include "raytracer.h"
#include "objloader.h"
#include "tutorials.h"

#include "options.h"
#include "Sampling.h"
#include "background.h"

clr3f defaultBackground = { 0.1f, 0.1f, 0.1f };
const vec3f p_light = Options::Get().omnilight_pos;
const vec3f lightClr = Options::Get().omnilight_clr;

Raytracer::Raytracer(const int width, const int height, const float fov_y, const vec3f view_from, const vec3f view_at, const char* config)
	: SimpleGuiDX11(width, height), background(std::make_unique<BackgroundStatic>(defaultBackground)) {
	InitDeviceAndScene(config);

	camera_ = Camera(width, height, fov_y, view_from, view_at);

	Options& opt = Options::Get();
	samples = opt.sampleCount;
	_1_samples = 1.f / samples;
	maxDepth = opt.maxDepth;
	Sampling::InitGenerator();
}

Raytracer::~Raytracer() {
	ReleaseDeviceAndScene();
}

//================================================================================================================================================

clr4f Raytracer::get_pixel(const int x, const int y, const float t) {
	clr3f pixel = { 0.f, 0.f, 0.f };
	RTCRay primaryRay;
	float fx = (float)x;
	float fy = (float)y;

	for (int i = 0; i < samples-1; i++) {
		primaryRay = camera_.GenerateRay(fx + Sampling::Random05(), fy + Sampling::Random05());
		pixel += TraceRay(primaryRay);
	}
	primaryRay = camera_.GenerateRay(fx, fy);
	pixel += TraceRay(primaryRay);

	return pixel * _1_samples;
}

clr3f Raytracer::TraceRay(RTCRay& ray, int depth, float n1) {
	clr3f color = { 0.f, 0.f, 0.f };

	IntersectionEmbree data = scene.IntersectRay(ray);
	if (data.IntersectionFailed() || depth >= maxDepth) {
		return background->GetPixelColor(data.v_rayDir);
	}
	else {
		data.PrepareData(scene);

		//phong - diffuse part
		clr3f clrPhong = data.clrDiffuse * data.dotNormalView + data.clrAmbient;

		//phong - specular part
		vec3f v_light = (p_light - data.p_rayHit).normalize();
		float dotNormalLight = data.v_normal.DotProduct(v_light);
		vec3f v_lightReflected = (2.f * dotNormalLight) * data.v_normal - v_light;
		float specComp = powf(max(data.v_view.DotProduct(v_lightReflected), 0), data.material->shininess);
		clrPhong += data.clrSpecular * specComp;

		//fresnel variables
		float R = 1.f;
		float n2 = n1 == 1.f ? data.material->ior : 1.f;
		float iorRatio = n1 / n2;
		float r0 = (n1 - n2) / (n1 + n2); r0 *= r0;

		//refraction
		float refractionRootVal = 1 - (iorRatio * iorRatio) * (1 - data.dotNormalRay * data.dotNormalRay);
		if (refractionRootVal >= 0) {
			vec3f v_rayDirRefracted = (iorRatio * data.v_rayDir - (iorRatio * data.dotNormalRay + sqrtf(refractionRootVal)) * data.v_normal).normalize();
			float dotNormalRayRefracted = data.v_normal.DotProduct(-v_rayDirRefracted);

			//fresnel (schlick) - reflected/refracted ratio
			float theta = n1 <= n2 ? data.dotNormalView : dotNormalRayRefracted;
			float minTheta = 1 - theta;
			R = r0 + (1 - r0) * minTheta * minTheta * minTheta * minTheta * minTheta;

			//only do refraction for glass surfaces
			if (data.material->shader == ShaderType::Glass) {
				RTCRay rayRefracted = PrepareRay(data.p_rayHit, v_rayDirRefracted);
				color = TraceRay(rayRefracted, depth + 1, n2) * (1.f - R);
			}
		}

		//reflection
		RTCRay rayReflected = PrepareRay(data.p_rayHit, data.v_rayDirReflected);
		color += TraceRay(rayReflected, depth + 1, n1)* R;

		//beer-lambert attenuation
		float l = n1 == 1.f ? 0.f : data.rhit.ray.tfar;
		clr3f T_bl = data.clrAttenuation * -l;
		T_bl = { expf(T_bl.r), expf(T_bl.g), expf(T_bl.b) };
		color *= T_bl;

		//final color
		if (data.material->shader != ShaderType::Glass)
			color += clrPhong;
	}

	return color;
}






//================================================================================================================================================

RTCRay Raytracer::PrepareRay(vec3f& rOrg, vec3f& rDir) {
	RTCRay ray = RTCRay();
	rOrg.SetupFromThis(ray.org_x, ray.org_y, ray.org_z);
	rDir.SetupFromThis(ray.dir_x, ray.dir_y, ray.dir_z);

	ray.tnear = 0.1f;
	ray.tfar = FLT_MAX;

	ray.flags = 0;
	ray.mask = 0;
	ray.id = 0;


	return ray;
}

int Raytracer::InitDeviceAndScene(const char* config) {
	device_ = rtcNewDevice(config);
	error_handler(nullptr, rtcGetDeviceError(device_), "Unable to create a new device.\n");
	rtcSetDeviceErrorFunction(device_, error_handler, nullptr);

	ssize_t triangle_supported = rtcGetDeviceProperty(device_, RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED);

	// create a new scene bound to the specified device
	scene.scene = rtcNewScene(device_);
	rtcSetSceneFlags(scene.scene, RTC_SCENE_FLAG_ROBUST);

	return S_OK;
}

int Raytracer::ReleaseDeviceAndScene() {
	rtcReleaseScene(scene.scene);
	rtcReleaseDevice(device_);

	return S_OK;
}

void Raytracer::LoadScene(const std::string file_name) {
	const int no_surfaces = LoadOBJ(file_name.c_str(), surfaces_, materials_);

	// surfaces loop
	for (auto surface : surfaces_) {
		RTCGeometry mesh = rtcNewGeometry(device_, RTC_GEOMETRY_TYPE_TRIANGLE);

		Vertex3f* vertices = (Vertex3f*)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
			sizeof(Vertex3f), 3 * surface->no_triangles());

		Triangle3ui* triangles = (Triangle3ui*)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
			sizeof(Triangle3ui), surface->no_triangles());

		rtcSetGeometryUserData(mesh, (void*)(surface->get_material()));

		rtcSetGeometryVertexAttributeCount(mesh, 2);

		Normal3f* normals = (Normal3f*)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3,
			sizeof(Normal3f), 3 * surface->no_triangles());

		Coord2f* tex_coords = (Coord2f*)rtcSetNewGeometryBuffer(
			mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT2,
			sizeof(Coord2f), 3 * surface->no_triangles());

		// triangles loop
		for (int i = 0, k = 0; i < surface->no_triangles(); ++i) {
			Triangle& triangle = surface->get_triangle(i);

			// vertices loop
			for (int j = 0; j < 3; ++j, ++k) {
				const Vertex& vertex = triangle.vertex(j);

				vertices[k].x = vertex.position.x;
				vertices[k].y = vertex.position.y;
				vertices[k].z = vertex.position.z;

				normals[k].x = vertex.normal.x;
				normals[k].y = vertex.normal.y;
				normals[k].z = vertex.normal.z;

				tex_coords[k].u = vertex.texture_coords[0].u;
				tex_coords[k].v = vertex.texture_coords[0].v;
			} // end of vertices loop

			triangles[i].v0 = k - 3;
			triangles[i].v1 = k - 2;
			triangles[i].v2 = k - 1;
		} // end of triangles loop

		rtcCommitGeometry(mesh);
		unsigned int geom_id = rtcAttachGeometry(scene.scene, mesh);
		rtcReleaseGeometry(mesh);
	} // end of surfaces loop

	rtcCommitScene(scene.scene);
}

int Raytracer::Ui() {
	static float f = 0.0f;
	static int counter = 0;

	// Use a Begin/End pair to created a named window
	ImGui::Begin("Ray Tracer Params");

	ImGui::Text("Surfaces = %d", surfaces_.size());
	ImGui::Text("Materials = %d", materials_.size());
	ImGui::Separator();
	ImGui::Checkbox("Vsync", &vsync_);

	//ImGui::Checkbox( "Demo Window", &show_demo_window ); // Edit bools storing our window open/close state
	//ImGui::Checkbox( "Another Window", &show_another_window );

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f    
	//ImGui::ColorEdit3( "clear color", ( float* )&clear_color ); // Edit 3 floats representing a color

	// Buttons return true when clicked (most widgets return true when edited/activated)
	if (ImGui::Button("Button"))
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	// 3. Show another simple window.
	/*if ( show_another_window )
	{
	ImGui::Begin( "Another Window", &show_another_window ); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Text( "Hello from another window!" );
	if ( ImGui::Button( "Close Me" ) )
	show_another_window = false;
	ImGui::End();
	}*/

	return 0;
}
