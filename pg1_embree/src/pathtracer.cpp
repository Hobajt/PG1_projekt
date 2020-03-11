#include "stdafx.h"
#include "pathtracer.h"
#include "objloader.h"
#include "tutorials.h"

#include "options.h"
#include "Sampling.h"
#include "background.h"

clr3f Pathtracer::defaultBackground = { 0.0f, 0.0f, 0.0f };

Pathtracer::Pathtracer(const int width, const int height, const float fov_y, const vec3f view_from, const vec3f view_at, const char* config)
	: SimpleGuiDX11(width, height), background(std::make_unique<BackgroundStatic>(defaultBackground)) {
	InitDeviceAndScene(config);

	camera_ = Camera(width, height, fov_y, view_from, view_at);

	Options& opt = Options::Get();
	samples = opt.sampleCount;
	_1_samples = 1.f / samples;
	maxDepth = opt.maxDepth;
	Sampling::InitGenerator();
	IntersectionEmbree::convertMaterials = opt.materialToLinear;
}

Pathtracer::~Pathtracer() {
	ReleaseDeviceAndScene();
}

//================================================================================================================================================

clr4f Pathtracer::get_pixel(const int x, const int y, const float t) {
	clr3f pixel = { 0.f, 0.f, 0.f };
	RTCRay primaryRay;
	float fx = (float)x;
	float fy = (float)y;

	for (int i = 0; i < samples - 1; i++) {
		primaryRay = camera_.GenerateRay(fx + Sampling::Random05(), fy + Sampling::Random05());
		pixel += TraceRay(primaryRay);
	}
	primaryRay = camera_.GenerateRay(fx, fy);
	pixel += TraceRay(primaryRay);

	return pixel * _1_samples;
}

clr3f Pathtracer::TraceRay(RTCRay& ray, int depth, float n1) {
	clr3f color = { 0.f, 0.f, 0.f };

	IntersectionEmbree data = scene.IntersectRay(ray);
	if (data.IntersectionFailed() || depth >= maxDepth) {
		return background->GetPixelColor(data.v_rayDir);
	}
	else {
		data.PrepareData(scene);

		if (!data.clrEmission.IsZero())
			color = data.clrEmission;
		else {
			HemisphereSample sample;
			clr3f fr = clr3f{ 1.f, 1.f, 1.f };
			float PDF = 1.f;

			switch (data.material->shader) {
				default:
				case ShaderType::Lambert:
					sample = Sampling::CosWeighted(data.v_normal);

					//Lambert BRDF
					fr = data.clrDiffuse * M_1_PI;
					break;
				/*case ShaderType::Phong:
					break;
				case ShaderType::Glass:
					break;*/
				case ShaderType::Mirror:
					sample.omegaI = data.v_rayDirReflected;
					sample.dotNormalOmegaI = sample.PDF = 1.f;
					break;
			}

			
			
			PDF *= sample.PDF;
			RTCRay rayReflected = PrepareRay(data.p_rayHit, sample.omegaI);
			color = TraceRay(rayReflected, depth + 1, n1) * fr * sample.dotNormalOmegaI * (1.f / PDF);
		}

	}

	return color;
}

bool Pathtracer::IsLightVisible(IntersectionEmbree& data, vec3f& v_light) {
	RTCRay shadowRay = PrepareRay(data.p_rayHit, v_light);
	RTCRayHit rhit = SceneData::SetupRayHitStructure(shadowRay);

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(scene.scene, &context, &rhit);

	return rhit.hit.geomID == RTC_INVALID_GEOMETRY_ID;
}




//================================================================================================================================================

RTCRay Pathtracer::PrepareRay(vec3f& rOrg, vec3f& rDir) {
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

int Pathtracer::InitDeviceAndScene(const char* config) {
	device_ = rtcNewDevice(config);
	error_handler(nullptr, rtcGetDeviceError(device_), "Unable to create a new device.\n");
	rtcSetDeviceErrorFunction(device_, error_handler, nullptr);

	ssize_t triangle_supported = rtcGetDeviceProperty(device_, RTC_DEVICE_PROPERTY_TRIANGLE_GEOMETRY_SUPPORTED);

	// create a new scene bound to the specified device
	scene.scene = rtcNewScene(device_);
	rtcSetSceneFlags(scene.scene, RTC_SCENE_FLAG_ROBUST);

	return S_OK;
}

int Pathtracer::ReleaseDeviceAndScene() {
	rtcReleaseScene(scene.scene);
	rtcReleaseDevice(device_);

	return S_OK;
}

void Pathtracer::LoadScene(const std::string file_name) {
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

int Pathtracer::Ui() {
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
